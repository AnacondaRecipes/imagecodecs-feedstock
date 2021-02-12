import os
import platform
import sys
from distutils.version import LooseVersion

def customize_build(EXTENSIONS, OPTIONS):
    """Customize build for conda-forge."""
    del EXTENSIONS['jpeg12']
    del EXTENSIONS['avif']

    # build jpeg8 or jpeg9 against libjpeg instead of libjpeg_turbo
    OPTIONS['cythonize'] = True
    EXTENSIONS['jpeg8']['cython_compile_env']['HAVE_LIBJPEG_TURBO'] = False
    EXTENSIONS['lerc']['libraries'] = ['Lerc']

    if sys.platform == 'win32':
        # Windows build of brunsli seem pretty experimental
        # Windows builds seem too experimental upstream
        # https://github.com/google/brunsli/issues/51
        # https://github.com/google/brunsli/issues/62
        # https://github.com/google/brunsli/issues/93
        del EXTENSIONS['jpegxl']
        library_inc = os.environ.get('LIBRARY_INC', '')
        EXTENSIONS['bz2']['libraries'] = ['bzip2']
        EXTENSIONS['jpeg2k']['include_dirs'] += [
            os.path.join(
                library_inc, 'openjpeg-' + os.environ['openjpeg']
            )
        ]
        EXTENSIONS['deflate']['libraries'] = ['libdeflate']
        EXTENSIONS['jpegls']['libraries'] = ['charls-2-x64']
        EXTENSIONS['lz4']['libraries'] = ['liblz4']
        EXTENSIONS['lz4f']['libraries'] = ['liblz4']
        EXTENSIONS['lzma']['libraries'] = ['liblzma']
        EXTENSIONS['png']['libraries'] = ['libpng', 'z']
        EXTENSIONS['webp']['libraries'] = ['libwebp']
        EXTENSIONS['jpegxr']['include_dirs'] = [
            os.path.join(os.environ['LIBRARY_INC'], 'jxrlib')
        ]
        EXTENSIONS['jpegxr']['libraries'] = ['libjpegxr', 'libjxrglue']
    else:
        EXTENSIONS['jpegxr']['include_dirs'] = [
            os.path.join(os.environ['PREFIX'], 'include', 'jxrlib')
        ]
        EXTENSIONS['jpegxr']['libraries'] = ['jpegxr', 'jxrglue']

    if platform.machine() == 's390x':
        # These extensions seem to be broken and trigger test failures S390X,
        # even though the libraries they use pass all their tests.
        del EXTENSIONS['lerc']      # all tests fail
        del EXTENSIONS['zfp']       # all tests fail
        del EXTENSIONS['jpegls']    # encode/decode tests pass; round-trip tests fail
        del EXTENSIONS['jpegxl']    # encode/decode tests fail; round-trip tests pass
