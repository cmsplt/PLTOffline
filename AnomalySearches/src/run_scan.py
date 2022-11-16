# import rich
import sys
import pathlib
import os
import glob
import subprocess
import argparse
import joblib


class SlincAnalyzerCaller:
    def __init__(self) -> None:
        self.libname = (
            pathlib.Path(__file__).parent.absolute().parent.absolute()
            / "KmeansAnalyzer"
        )
        if not self.libname.exists() and glob.glob(self.libname + "main.o"):
            print("Must compile KmeansAnalyzer first")
            raise Exception("KmeansAnalyzer not found or not compiled")

        self.keys = [
            "fit params",
            "maskedMxAvg",
            "maskedMxStd",
            "std",
            "nstd",
            "classIndex",
            "isNormal",
            "fractionOfHitsOutsideOfActiveArea",
        ]

    def _clean_output(self, output: str) -> dict:
        """
        Extracts data as a dictionary from the output of the analyzer
        Args:
            output (str): fprocess.stdout

        """

        out_lines = output.split("\n")

        return {
            l.split(":")[0].strip(): l.split(":")[1].strip()
            for l in out_lines
            if l.split(":")[0].strip() in self.keys
        }

    def run_file(self, file_name: str) -> dict:
        """Runs the analyzer on a single file
        Args:
            file_name (str): _description_
        Returns:
            dict: keys of the analyzed file
        """
        self.in_file = file_name
        assert os.path.exists(self.in_file)
        fprocess = subprocess.run(
            f"{self.libname.absolute()}/./main  {self.libname.absolute()}/input {self.in_file}",
            text=True,
            shell=True,
            stdout=subprocess.PIPE,
        )
        putput = fprocess.stdout
        return self._clean_output(putput)

    def scan_folder(self, folder):
        files = glob.glob(folder + "/*.root")
        return {f: self.run_file(f) for f in files}


def parse_input():

    parser = argparse.ArgumentParser(
        description="Runs the analyzer on a folder or a single file",
    )
    parser.add_argument(
        "target",
        type=str,
        help="Folder or File of root files (single file) to be analyzed",
        nargs="?",
        default="",
    )
    return parser.parse_args()


if __name__ == "__main__":
    analyzer = SlincAnalyzerCaller()
    folder, file = None, None
    arg_in = parse_input().target
    if arg_in:
        print("Analyzing", arg_in)
        if os.path.isdir(arg_in):
            folder = arg_in
            print("Scanning folder", folder)
        elif os.path.isfile(arg_in):
            file = arg_in
            print("Scanning file", folder)
        else:
            print("Invalid input, please porovide a folder or a root file")
            sys.exit(1)

    else:
        print("No input provided")
        arg_in = input("Enter folder or file to be analyzed: ")
        if os.path.isdir(arg_in):
            folder = arg_in
            print("Scanning folder", folder)
        elif os.path.isfile(arg_in):
            file = arg_in
            print("Scanning file", folder)
        else:
            print("Invalid input, please porovide a folder or a root file")
            sys.exit(1)

    if file:
        out = analyzer.run_file(
            pathlib.Path(file).absolute().as_posix(),
        )
    elif folder:
        out = analyzer.scan_folder(
            pathlib.Path(folder).absolute().as_posix(),
        )
    joblib.dump(out, "./output.pkl")
    print("Done, output saved to ./output.pkl")
    print(out)
