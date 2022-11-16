# import rich
import sys
import pathlib
import os
import glob
import subprocess


class SlincAnalyzerCaller:
    def __init__(self) -> None:
        self.libname = (
            pathlib.Path(__file__).parent.absolute().parent.absolute()
            / "KmeansAnalyzer"
        )
        if not self.libname.exists() and glob.glob(self.libname + "main.o"):
            print("Must compile KmeansAnalyzer first")
            raise Exception("KmeansAnalyzer not found or not compiled")

    def run_file(self, file_name):
        self.in_file = file_name
        assert os.path.exists(self.in_file)
        fprocess = subprocess.run(
            [f"./{self.libname}/main", "{self.in_file}"], capture_output=True, text=True
        )


if __name__ == "__main__":
    run_slink_analyzer()
