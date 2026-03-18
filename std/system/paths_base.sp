package std;
module pathsbase;

function text slash() {
    return "/";
}

function text build_dir() {
    return "build";
}

function text join_path(text left, text right) {
    sharp {
        text base = join(left, slash());
        return join(base, right);
    }
}

function text join_path3(text first, text second, text third) {
    sharp {
        text left = join_path(first, second);
        return join_path(left, third);
    }
}

function text build_file(text name) {
    return join_path(build_dir(), name);
}

function text html_in_build(text name) {
    sharp {
        text leaf = join(name, ".html");
        return build_file(leaf);
    }
}

function text txt_in_build(text name) {
    sharp {
        text leaf = join(name, ".txt");
        return build_file(leaf);
    }
}

function text log_in_build(text name) {
    sharp {
        text leaf = join(name, ".log");
        return build_file(leaf);
    }
}
