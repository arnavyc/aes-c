def ret_emp_str_if_none(a):
    if a is None:
        return ""

    return a


def get_version_from_file(version_file):
    with open(version_file) as f:
        content = f.read()
        a = content.split("*")
        version_dict = {
            "VERSION_STRING": a[0],
            "VERSION_STRING_FULL": a[1],
            "VERSION_MAJOR": int(a[2]) if a[2] != "" else 0,
            "VERSION_MINOR": int(a[3]) if a[2] != "" else 0,
            "VERSION_PATCH": int(a[4]) if a[2] != "" else 0,
            "VERSION_TWEAK": a[5],
            "VERSION_AHEAD": int(a[6]) if a[6] != "" else 0,
            "VERSION_GIT_SHA": a[7],
        }
        return version_dict

