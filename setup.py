from setuptools import setup, Extension
import subprocess

module = Extension(
  '_pyhp',
  ['_pyhp.c'],
  libraries=['php5'],
  extra_compile_args=subprocess.check_output(['php-config', '--includes']).split(' '),
  extra_link_args=subprocess.check_output(['php-config', '--ldflags']).strip().split(' ') + \
                  subprocess.check_output(['php-config', '--libs']).strip().split(' '),
)

setup(
  name='pyhp',
  py_modules=['pyhp'],
  ext_modules=[module],
)
