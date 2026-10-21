# Vulkan backend notes

This branch is rdb's `vulkan` branch (eeb376e855) merged into this fork's
master, plus the fixes listed at the bottom.

## Build

```sh
# Arch: vulkan-headers vulkan-icd-loader glslang spirv-tools spirv-cross (no extra SDK needed)
python3 makepanda/makepanda.py --everything --use-vulkan --no-eigen \
    --threads 6 --outputdir built --optimize 3
```

The display module is `built/lib/libp3vulkandisplay.so`.

## Enabling it

```
load-display p3vulkandisplay
sync-video #f          # uncapped: MAILBOX if available, else IMMEDIATE, >=4 swapchain images
```

Put this in a prc page that is loaded after any page that sets another
`load-display`. There is no `aux-display` fallback, so a broken Vulkan setup
fails loudly instead of silently going back to GL.

| prc | default | |
|---|---|---|
| `vulkan-present-mode` | `auto` | `fifo`, `fifo-relaxed`, `mailbox`, `immediate`; unsupported falls back to fifo |
| `vulkan-swapchain-images` | `0` (auto) | auto = 1 + back-buffers, at least 4 when uncapped |
| `vulkan-validation` | `#f` | enable `VK_LAYER_KHRONOS_validation` (install `vulkan-validation-layers`); slow |
| `vulkan-frame-stats` | `0` | every N frames log uploads/allocs/descriptor allocs and fence/acquire/present waits (needs `notify-level-vulkandisplay info`) |
| `vulkan-recycle-semaphores` | `#t` | reuse per-frame acquire semaphores |
| `vulkan-keep-last-frame` | `#f` | always keep a copy of the last frame for screenshots (see below) |
| `vulkan-persist-pipeline-cache` | `#t` | load/save the VkPipelineCache under `model-cache-dir` |
| `vulkan-pipeline-cache-file` | | override the pipeline cache path |

The chosen present mode and image count are logged at info level
(`Swapchain 1280x720 present mode immediate, 4 images requested ...`). On
NVIDIA under Xwayland the surface offers no MAILBOX, so uncapped means
IMMEDIATE. With 3 images, IMMEDIATE showed `vkAcquireNextImageKHR` stalls of
~6 ms in bursts (bad 1% lows); 4 images removes them, 5 is no better.

**Linux/Wayland:** the Vulkan window is an X11 (Xwayland) window, so `DISPLAY`
must point at the Xwayland server. Without a usable display you get
`Cannot open a Vulkan window: no X11 display connection (check DISPLAY / XAUTHORITY)`
and "Could not open window" instead of a segfault.

`threading-model Cull/Draw` works on Vulkan.

## Custom shaders

Write normal GLSL (`#version 330`+) using Panda's `p3d_*` inputs. Panda
compiles it with glslang to SPIR-V, and the same shader runs unchanged on
OpenGL and Vulkan:

```python
from panda3d.core import Shader, Vec4
sh = Shader.load(Shader.SL_GLSL, "myshader.vert", "myshader.frag")
np.setShader(sh, 10)
np.setShaderInput("tint", Vec4(1, 0.5, 0.2, 1))
```

Post-processing works through `direct.filter.FilterManager`. The quad shader
must take a `texpad_tex` input, `(0.5*(w-padx)/w, 0.5*(h-pady)/h)`, because
offscreen textures are power-of-two padded.

Tips: declare every uniform you read, and set a default value for each custom
input *before* the first frame (GL asserts on a missing input). Avoid relying
on implicit varyings. `p3d_ColorScale`, `p3d_Texture0`,
`p3d_ModelViewProjectionMatrix`, `p3d_NormalMatrix` etc. work on both backends.

## Shader and pipeline caches

Two layers, both under `model-cache-dir` (`$XDG_CACHE_HOME/panda3d` by default):

| Cache | File | Keyed by | Portable? |
|---|---|---|---|
| Compiled shaders (SPIR-V) | `generated-shaders/<hash>.sho` | 128-bit hash of shader source + language + format tag | Yes, GPU independent; can be built once and shipped |
| Vulkan pipelines | `vulkan-pipeline-cache.bin` | `VkPipelineCacheHeaderVersionOne` (vendorID, deviceID, pipelineCacheUUID) | No, per GPU/driver; a stale file is ignored and rebuilt |

* Generated shaders (ShaderGenerator / `Shader.make`) are compiled once, then
  loaded from disk. Prc: `cache-generated-shaders-on-disk #t`. Bump the
  `gensho-N` format tag in `Shader::make` whenever the compiler or SPIR-V
  passes change incompatibly.
* Shaders loaded from files go through Panda's existing `.sho` cache when
  `model-cache-compiled-shaders #t`.
* The pipeline cache is loaded when the GSG is created and saved atomically in
  `close_gsg()`. It is only written when the window is closed normally
  (`base.graphicsEngine.removeAllWindows()` / normal exit); `os._exit()` skips it.

`samples/vulkan-cache/shader_precache.py` renders a model list (and optional
GLSL shader pairs) offscreen to fill both caches. An application can ship the
`.sho` files and run it as a first-launch "pre-compiling shaders" step; a
driver/GPU change needs nothing special, since a pipeline cache whose header
doesn't match is discarded and rebuilt.

## Screenshots (spec-correct readback)

A presented swapchain image may not be accessed until it is acquired again.
`saveScreenshot()` / `getScreenshot()` (an FM_refresh pass) therefore:

1. reads the last rendered swapchain image directly **only while it is still
   unpresented** (`_flip_ready` and it is the current image). This is the
   normal case: Panda flips at the start of the next `renderFrame()`, so a
   screenshot taken from a task or between frames costs nothing extra;
2. otherwise reads a private copy of the last frame (`<window>:last-frame`)
   that `end_frame()` copies from the swapchain image before the PRESENT_SRC
   transition. The copy is only made when it can be needed: with
   `auto-flip #t`, with `vulkan-keep-last-frame #t`, or after the first
   screenshot request that found no unpresented image (that request returns
   false and logs it; later frames keep a copy). While enabled it costs one
   full-window image copy per frame.

FM_refresh no longer runs into the expose / pending-configure early-outs in
`begin_frame()` and never recreates the swapchain.

## Validation

Run with `vulkan-validation #t`. Errors found and fixed this way:

| VUID | Cause | Fix |
|---|---|---|
| `vkCmdDraw*-dynamicRenderingUnusedAttachments-08917` | `framebuffer-stencil #t` gives a D24S8 attachment but the pipeline had `stencilAttachmentFormat` UNDEFINED | set stencil format for combined depth/stencil |
| `VkPresentInfoKHR-pImageIndices-01430` | screenshot left the presented swapchain image in TRANSFER_SRC | restore PRESENT_SRC after the readback |
| `vkDestroyDevice-device-05137` (VkSemaphore) | acquire semaphore of a failed (out-of-date) acquire overwritten on swapchain recreate | destroy it in `destroy_swapchain()` |
| `VkBufferCreateInfo-size-00912`, `vkCmdCopyBuffer-size-00116/-dstOffset-00114`, `vkCmdBindVertexBuffers-pOffsets-00626`, `vkCmdDrawIndexed-robustBufferAccess2-08798` (Cull/Draw) | vertex/index buffers sized at prepare time and never regrown; empty arrays made 0-byte buffers | regrow on size change, minimum 16 bytes |
| image view usage (kept last-frame copy) | transfer-only image got a view | create it with COLOR_ATTACHMENT usage too |

## Hangs fixed

* **FM_refresh / open_windows hang**: the GSG has a 40-entry command-buffer
  pool; when one frame needed more (many offscreen buffers, or
  `make_contexts()` doing FM_refresh on every window), `create_command_buffer()`
  waited for the oldest frame, which was the unsubmitted current frame,
  forever. Now it only waits for submitted frames and otherwise grows the pool.
  Repro: `samples/vulkan-cache/refresh_repro.py` (N offscreen buffers +
  openWindows + renderFrame + saveScreenshot) hung at buffer 39, now fine with N=100.
* **Cull/Draw screenshot deadlock** (GL too): `DisplayRegion::get_screenshot()`
  run on the draw thread re-entered `get_screenshot()`, failed the
  pipeline-stage check (run_on_thread lends the caller's stage) and called
  `run_on_draw_thread()` again, blocking on the engine lock. The Vulkan window
  screenshot also used the GSG's last-bound framebuffer (null on the draw
  thread) instead of the window's last rendered image.
* **`sys.exit()` hang after `removeAllWindows()`** (GL and Vulkan):
  `GraphicsEngine::remove_all_windows()` calls `AsyncTaskManager::stop_threads()`,
  which put the default task chain back to `S_initial` while it was still in
  `poll()`. When the running task then raised `SystemExit` (DS_interrupt), the
  chain only honoured that in `S_started`, so it kept running later tasks with
  the exception pending; one of them (`messenger.send`) left `Messenger.lock`
  held and `ShowBase.exitfunc` -> `Messenger.ignoreAll` deadlocked.
  `AsyncTaskChain` now honours DS_interrupt in `S_initial` too. Repro: a task
  that calls `removeAllWindows()` then `sys.exit(0)`.

## Changes on this branch

1. Clean error instead of a segfault when the X display can't be opened.
2. VkPipelineCache persisted to disk with header validation and atomic writes,
   plus pipeline-creation timing logs.
3. On-disk cache for generated / string-made shaders (`generated-shaders/*.sho`).
4. `samples/vulkan-cache/`: `shader_precache.py`, `refresh_repro.py`.
5. Validation-layer fixes (stencil format, present layout, semaphore leaks)
   and the `vulkan-validation` switch.
6. Swapchain: >=4 images when uncapped, `vulkan-present-mode`,
   `vulkan-swapchain-images`, maxImageCount=0 clamp fix, recycled semaphores,
   `vulkan-frame-stats`.
7. Command-buffer pool growth (FM_refresh / many-buffers hang).
8. Cull/Draw: screenshot deadlock fix (display), correct screenshot source,
   vertex/index buffer regrow.
9. Spec-correct screenshot readback (unpresented image or kept last-frame
   copy, `vulkan-keep-last-frame`); FM_refresh no longer gated.
10. Event: DS_interrupt honoured after `stop_threads()` (sys.exit hang fix).
