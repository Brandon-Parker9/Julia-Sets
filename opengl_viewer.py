import contextlib, sys
from OpenGL import *
import glfw

@contextlib.contextmanager
def create_main_window():
    if not glfw.init():
        sys.exit(1)
    try:
        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
        glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, True)
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

        title = 'Tutorial 2: First Triangle'
        window = glfw.create_window(500, 400, title, None, None)
        if not window:
            sys.exit(2)
        glfw.make_context_current(window)

        glfw.set_input_mode(window, glfw.STICKY_KEYS, True)
        gl.glClearColor(0, 0, 0.4, 0)

        yield window

    finally:
        glfw.terminate()

if __name__ == '__main__':
    with create_main_window() as window:
        while (
            glfw.get_key(window, glfw.KEY_ESCAPE) != glfw.PRESS and
            not glfw.window_should_close(window)
        ):
            gl.glClear(gl.GL_COLOR_BUFFER_BIT)
            glfw.swap_buffers(window)
            glfw.poll_events()