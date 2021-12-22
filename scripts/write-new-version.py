#!/usr/bin/env python3

import argparse
import sys
import re

semver_regex = r"^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$"


def ret_emp_str_if_none(a):
    if a is None:
        return ""

    return a


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("semver_version", help="SemVer version to use.")
    parser.add_argument(
        "--version-file",
        dest="version_file",
        help="Name of file to write version to.",
        default="VERSION",
    )

    args = parser.parse_args()
    matches = re.search(semver_regex, args.semver_version)

    with open(args.version_file, "w") as f:
        # Full version string
        f.write(ret_emp_str_if_none(args.semver_version))
        f.write("*")

        # Version string
        f.write(ret_emp_str_if_none(args.semver_version))
        f.write("*")

        # Major version
        f.write(ret_emp_str_if_none(matches.group("major")))
        f.write("*")

        # Minor version
        f.write(ret_emp_str_if_none(matches.group("minor")))
        f.write("*")

        # Patch version
        f.write(ret_emp_str_if_none(matches.group("patch")))
        f.write("*")

        # Tweak (prerelease) version
        f.write(ret_emp_str_if_none(matches.group("prerelease")))
        f.write("*")

        # Commits ahead of tag = 0
        f.write("0*")

        # Git_SHA (No need to add)
    return 0


if __name__ == "__main__":
    sys.exit(main())
