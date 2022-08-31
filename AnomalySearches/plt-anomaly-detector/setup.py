from setuptools import setup, find_packages
import pathlib

HERE = pathlib.Path(__file__).parent

setup(
    name="plt-anomaly-detector",
    version="0.0.1",
    description="A proof of concept for anomaly detection in the PLT detector",
    author="Jose M Munoz, PLT Team",
    author_email="foomail@foo.example",
    install_requires=[],  # external packages as dependencies
    scripts=[
        "scripts/mount_fills.py",
    ],
    packages=find_packages(),
)
