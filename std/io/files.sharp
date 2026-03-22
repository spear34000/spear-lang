package std;
module files;

function text write_text_file(text path, text content) {
    write(path, content);
    return content;
}

function text write_line_file(text path, text content) {
    sharp {
        text body = join(content, "\n");
        write(path, body);
        return body;
    }
}
