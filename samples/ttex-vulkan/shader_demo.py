# Usage: python3 shader_demo.py [p3vulkandisplay|pandagl] [post|nopost]
# Hologram (rim) shader on the panda + FilterManager post-process; saves a screenshot and exits.
import sys, os
from panda3d.core import loadPrcFileData, Shader, Filename, ClockObject, Vec4
disp = sys.argv[1] if len(sys.argv) > 1 else 'p3vulkandisplay'
loadPrcFileData('', 'load-display %s\nwindow-title SHADERDEMO\nsync-video #f\nnotify-level-vulkandisplay info\n' % disp)
from direct.showbase.ShowBase import ShowBase
from direct.filter.FilterManager import FilterManager
D = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'shaders')
b = ShowBase()
print('PIPE', b.win.getGsg().getType().getName(), b.win.getGsg().getDriverRenderer())
env = b.loader.loadModel('models/environment'); env.reparentTo(b.render); env.setScale(0.25); env.setPos(-8, 42, 0)
p = b.loader.loadModel('models/panda'); p.reparentTo(b.render); p.setPos(0, 25, -2); p.setScale(0.5); p.setH(200); b.cam.setPos(0, 0, 2)
rim = Shader.load(Shader.SL_GLSL, Filename(D + '/hologram.vert'), Filename(D + '/hologram.frag'))
print('RIM shader loaded', rim is not None and not rim.getErrorFlag())
p.setShader(rim); p.setShaderInput('rim_color', Vec4(1.0, 0.55, 0.1, 0.9)); p.setShaderInput('rim_power', 2.0)
mode = sys.argv[2] if len(sys.argv) > 2 else 'post'
if mode == 'post':
    fm = FilterManager(b.win, b.cam); tex = b.loader.loadTexture  # noqa
    from panda3d.core import Texture
    t = Texture(); quad = fm.renderSceneInto(colortex=t)
    quad.setShader(Shader.load(Shader.SL_GLSL, Filename(D + '/post.vert'), Filename(D + '/post.frag'))); quad.setShaderInput('tex', t); quad.setShaderInput('texpad_tex', Vec4(0.5, 0.5, 0, 0))
    b.graphicsEngine.renderFrame()
    from panda3d.core import Vec4 as V4
    quad.setShaderInput('texpad_tex', V4(0.5 * t.getXSize() / max(1, t.getXSize() + t.getPadXSize()) if False else 0.5 * (t.getXSize() - t.getPadXSize()) / t.getXSize(), 0.5 * (t.getYSize() - t.getPadYSize()) / t.getYSize(), 0, 0))
    print('TEX', t.getXSize(), t.getYSize(), 'pad', t.getPadXSize(), t.getPadYSize())
    print('POST quad', quad)
def done(task):
    c = ClockObject.getGlobalClock(); print('FPS', c.getAverageFrameRate(), 'frames', c.getFrameCount())
    print('SHOT', b.win.saveScreenshot(Filename('shaderdemo_%s_%s.png' % (disp, mode))))
    b.graphicsEngine.removeAllWindows(); os._exit(0)
b.taskMgr.doMethodLater(6, done, 'done')
b.run()
