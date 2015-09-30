# -*- coding: utf-8 -*-

from setuptools import setup, find_packages, Extension
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

calltrace_extension = Extension("calltrace", sources=["calltrace/calltrace_module.c"])

setup(
    name='calltrace',
    version='0.0.2',

    description='sys._current_frames replacement that does not capture local variables',
    ext_modules = [calltrace_extension],

    author='Torsten Landschoff',
    author_email='torsten@landschoff.net',
    license='MIT',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
    ],

    keywords='traceback stacktrace _current_frames',
    packages=find_packages(exclude=['contrib', 'docs', 'tests*']),
    install_requires=[],

    extras_require={
        'test': ['pytest'],
    },
)
