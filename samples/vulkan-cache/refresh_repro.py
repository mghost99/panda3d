# Repro for the FM_refresh / make_contexts hang: many offscreen buffers, each
# followed by openWindows() (which runs make_contexts -> FM_refresh on every window),
# interleaved with rendered frames and screenshots.
import sys, os, time, faulthandler
if os.environ.get('RR_TRAP'):
    import threading, signal
    threading.Timer(float(os.environ['RR_TRAP']), lambda: os.kill(os.getpid(), signal.SIGINT)).start()
else:
    faulthandler.dump_traceback_later(int(os.environ.get('RR_TIMEOUT', '40')), exit=True)
from panda3d.core import loadPrcFileData, Texture, GraphicsOutput, FrameBufferProperties, WindowProperties, GraphicsPipe
loadPrcFileData('', 'load-display %s\nwin-size 640 480\nsync-video #f\nframebuffer-stencil #t\n%s' % (os.environ.get('RR_DISPLAY', 'p3vulkandisplay'), os.environ.get('RR_PRC', '').replace('\\n', '\n')))
from direct.showbase.ShowBase import ShowBase
b = ShowBase()
m = b.loader.loadModel('panda'); m.reparentTo(b.render); b.cam.setPos(0, -40, 5)
N = int(os.environ.get('RR_N', '40')); t0 = time.time()
bufs = []
for i in range(N):
    tex = Texture('t%d' % i)
    buf = b.win.makeTextureBuffer('buf%d' % i, 256, 256, tex)
    if buf is None: print('makeTextureBuffer failed', i); break
    cam = b.makeCamera(buf); cam.reparentTo(b.cam)
    bufs.append(buf)
    b.graphicsEngine.openWindows()      # make_contexts -> FM_refresh on all windows
    if i % 3 == 0: b.graphicsEngine.renderFrame()
    if i % 7 == 0 and not os.environ.get('RR_NOSHOT'): b.win.saveScreenshot('/tmp/rr_%d.png' % i)
    print('step', i, '%.2fs' % (time.time() - t0), flush=True)
for j in range(30): b.graphicsEngine.renderFrame()
print('RR_OK buffers=%d %.2fs renderer=%s' % (len(bufs), time.time() - t0, b.win.gsg.driver_renderer), flush=True)
os._exit(0)
