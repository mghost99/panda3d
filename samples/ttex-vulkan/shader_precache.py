#!/usr/bin/env python3
"""TTEX shader pre-cache ("Pre-compiling shaders..." step).

Renders a list of models offscreen so that Panda3D compiles every generated
shader (ShaderGenerator / Shader.make) and every custom GLSL shader to SPIR-V,
and so the Vulkan driver builds the matching pipelines.  Results land in:

  <model-cache-dir>/generated-shaders/*.sho   GPU-independent compiled shaders
                                               (can be built once and shipped)
  <model-cache-dir>/vulkan-pipeline-cache.bin  GPU/driver-specific VkPipelineCache
                                               (header-checked against vendor,
                                               device id and pipelineCacheUUID)

Usage (from the game root, so config/*.prc vfs-mounts resolve):
  python3 shader_precache.py --prc config/dev.prc --models models.txt \
      [--shaders DIR] [--display p3vulkandisplay]
models.txt: one model path per line (e.g. phase_4/models/neighborhoods/toontown_central).
"""
import argparse, os, sys, time, glob


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--prc', action='append', default=[])
    ap.add_argument('--models', help='file with one model path per line')
    ap.add_argument('--model', action='append', default=[])
    ap.add_argument('--shaders', help='dir of *.vert/*.frag pairs to precompile')
    ap.add_argument('--display', default='p3vulkandisplay')
    ap.add_argument('--frames', type=int, default=3)
    a = ap.parse_args()

    from panda3d.core import (loadPrcFile, loadPrcFileData, Filename, Shader,
                              BamCache, VirtualFileSystem, ConfigVariableList,
                              NodePath, Vec4)
    for p in a.prc:
        loadPrcFile(Filename.fromOsSpecific(p))
    loadPrcFileData('precache', 'load-display %s\nwindow-type offscreen\n'
                    'audio-library-name null\nnotify-level-vulkandisplay info\n' % a.display)
    vfs = VirtualFileSystem.getGlobalPtr()
    for m in ConfigVariableList('vfs-mount'):
        src, dst = (m.split(' ', 2) + [None, None])[:2]
        vfs.mount(Filename(src), Filename(dst), 0)

    from direct.showbase.ShowBase import ShowBase
    base = ShowBase()
    print('precache: GSG', base.win.getGsg().getType().getName(), base.win.getGsg().getDriverRenderer())
    cache = BamCache.getGlobalPtr()
    print('precache: cache root', cache.getRoot(), 'active', cache.getActive())

    models = list(a.model)
    if a.models:
        models += [l.strip() for l in open(a.models) if l.strip() and not l.startswith('#')]
    t0 = time.time(); n = 0
    for path in models:
        try:
            np = base.loader.loadModel(path)
        except Exception as e:
            print('precache: skip', path, e); continue
        np.reparentTo(base.render)
        # Frame the model so everything is inside the view frustum.
        b = np.getBounds()
        if not b.isEmpty():
            c, r = b.getCenter(), max(b.getRadius(), 0.1)
            base.camera.setPos(c[0], c[1] - r * 2.5, c[2] + r * 0.5); base.camera.lookAt(c)
        for _ in range(a.frames):
            base.graphicsEngine.renderFrame()
        np.removeNode(); n += 1
    if a.shaders:
        for vs in sorted(glob.glob(os.path.join(a.shaders, '*.vert'))):
            fs = vs[:-5] + '.frag'
            if not os.path.exists(fs):
                continue
            sh = Shader.load(Shader.SL_GLSL, Filename.fromOsSpecific(vs), Filename.fromOsSpecific(fs))
            card = base.loader.loadModel('models/box') if vfs.exists('models/box.egg') else NodePath('card')
            card.reparentTo(base.render); card.setShader(sh)
            card.setShaderInput('rim_color', Vec4(1)); card.setShaderInput('rim_power', 2.0)
            for _ in range(a.frames):
                base.graphicsEngine.renderFrame()
            card.removeNode(); print('precache: shader', os.path.basename(vs), 'ok' if not sh.getErrorFlag() else 'ERROR')
    print('precache: rendered %d models in %.1fs' % (n, time.time() - t0))
    base.graphicsEngine.removeAllWindows()  # writes vulkan-pipeline-cache.bin
    root = cache.getRoot().toOsSpecific()
    shos = glob.glob(os.path.join(root, 'generated-shaders', '*.sho'))
    pc = os.path.join(root, 'vulkan-pipeline-cache.bin')
    print('precache: %d generated shaders cached, pipeline cache %d bytes' % (len(shos), os.path.getsize(pc) if os.path.exists(pc) else 0))
    os._exit(0)


if __name__ == '__main__':
    main()
