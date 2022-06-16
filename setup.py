from distutils.core import setup, Extension

module1 = Extension('rlviewer',
                    sources = ['src/rlviewer.cpp',
                               'src/viewer.cpp',
                               'src/vboindexer.cpp',
                               'src/texture.cpp',
                               'src/shader.cpp',
                               'src/objloader.cpp',
                               'src/controls.cpp'],
                    include_dirs = ['libs/glm-0.9.7.1'],
                    libraries = ['glfw', 'GLEW'],
                    define_macros=[('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION')])

setup (name = 'rlviewer',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1])


