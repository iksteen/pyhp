from setuptools import setup, Extension
import subprocess


__version__ = '0.5.0'


module = Extension(
  '_pyhp',
  ['_pyhp.c', 'python_object_proxy.c', 'translate_python_value.c', 'translate_php_value.c'],
  libraries=['php5'],
  extra_compile_args=subprocess.check_output(['php-config', '--includes']).split(' '),
  extra_link_args=subprocess.check_output(['php-config', '--ldflags']).strip().split(' ') + \
                  subprocess.check_output(['php-config', '--libs']).strip().split(' '),
)

setup(
  name='pyhp',
  version=__version__,
  py_modules=['pyhp'],
  ext_modules=[module],
  description='PHP embedded in Python',
  author='Ingmar Steen',
  author_email='iksteen@gmail.com',
  url='https://github.com/iksteen/pyhp/',
  download_url='https://github.com/iksteen/pyhp/tarball/v%s' % __version__,
)
