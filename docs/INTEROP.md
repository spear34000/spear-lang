# Sharp Interop

Sharp can vendor Python and Node packages into a project and generate wrapper modules for them.

## Add Packages

```bash
sharp add pip requests
sharp add pip matplotlib
sharp add pip plotly
sharp add pip pillow
sharp add npm dayjs
sharp add npm axios
```

When you add a package, Sharp updates `sharp.toml` and creates:

- `.sharp/vendor/...`
- `.sharp/shims/...`
- `interop/<package>.sp`
- `interop/<package>_example.sp`
- `std/interop.sp`
- `std/json.sp`

## Generated Wrapper Style

Sharp keeps a low-level escape hatch:

```sp
import "std/interop.sp";

run {
    value module = pip_module("requests_sharp");
    print(py_call(module, "get_text", "..."));
}
```

But for common packages it also generates readable preset wrappers.

## Requests Preset

After:

```bash
sharp add pip requests
```

you can use:

```sp
import "interop/requests.sp";

run {
    print(get_text("https://example.com"));
    print(get_json("https://example.com/data.json"));
    print(status_code("https://example.com"));
}
```

Generated helpers:

- `get_text(url)`
- `get_json(url)`
- `post_json(url, body_json)`
- `status_code(url)`

## Numpy Preset

After:

```bash
sharp add pip numpy
```

you can use:

```sp
import "interop/numpy.sp";

run {
    print(array_sum("[1,2,3]"));
    print(mean("[1,2,3]"));
}
```

Generated helpers:

- `array_sum(numbers_json)`
- `mean(numbers_json)`

## Pandas Preset

After:

```bash
sharp add pip pandas
```

you can use:

```sp
import "interop/pandas.sp";

run {
    print(read_csv_head("data.csv", 5));
    print(columns("data.csv"));
}
```

Generated helpers:

- `read_csv_head(path, rows)`
- `columns(path)`

## Matplotlib Preset

After:

```bash
sharp add pip matplotlib
```

you can use:

```sp
import "interop/matplotlib.sp";

run {
    print(line_svg("Demo", "[\"Mon\",\"Tue\",\"Wed\"]", "[4,7,5]"));
}
```

Generated helpers:

- `line_svg(title, labels_json, values_json)`
- `bar_svg(title, labels_json, values_json)`

## Plotly Preset

After:

```bash
sharp add pip plotly
```

you can use:

```sp
import "interop/plotly.sp";

run {
    print(bar_html("Demo", "[\"Mon\",\"Tue\",\"Wed\"]", "[4,7,5]"));
}
```

Generated helpers:

- `bar_html(title, labels_json, values_json)`
- `line_html(title, labels_json, values_json)`

## Pillow Preset

After:

```bash
sharp add pip pillow
```

you can use:

```sp
import "interop/pillow.sp";

run {
    print(image_size("input.png"));
    print(thumbnail("input.png", 320, 240, "thumb.png"));
}
```

Generated helpers:

- `image_size(path)`
- `thumbnail(path, width, height, output_path)`

## Dayjs Preset

After:

```bash
sharp add npm dayjs
```

you can use:

```sp
import "interop/dayjs.sp";

run {
    print(format_now("YYYY-MM-DD"));
    print(add_days("2026-03-22T00:00:00Z", 7, "YYYY-MM-DD"));
    print(from_iso("2026-03-22T12:30:00Z", "YYYY/MM/DD"));
}
```

Generated helpers:

- `format_now(pattern)`
- `add_days(iso_value, days, pattern)`
- `from_iso(iso_value, pattern)`

## Axios Preset

After:

```bash
sharp add npm axios
```

you can use:

```sp
import "interop/axios.sp";

run {
    print(get_text("https://example.com"));
    print(get_json("https://example.com/data.json"));
    print(status_code("https://example.com"));
}
```

Generated helpers:

- `get_text(url)`
- `get_json(url)`
- `post_json(url, body_json)`
- `status_code(url)`

## Project Layout

Typical project output after adding packages:

```text
my-app/
  sharp.toml
  main.sp
  interop/
    requests.sp
    matplotlib.sp
    plotly.sp
    pillow.sp
    dayjs.sp
    axios.sp
    requests_example.sp
    numpy_example.sp
  std/
    interop.sp
    json.sp
  .sharp/
    vendor/
    shims/
```

## Design Goal

Sharp interop is meant to feel higher-level than raw Python or raw JavaScript embedding:

- keep package installation project-local
- generate importable Sharp modules
- expose common operations as normal Sharp functions
- preserve low-level `py_call` and `js_call` escape hatches
