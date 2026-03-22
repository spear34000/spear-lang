# Sharp Interop

Sharp can vendor Python and Node packages into a project and generate wrapper modules for them.

## Add Packages

```bash
sharp add pip requests
sharp add npm dayjs
sharp add npm axios
```

When you add a package, Sharp updates `sharp.toml` and creates:

- `.sharp/vendor/...`
- `.sharp/shims/...`
- `interop/<package>.sp`
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
    dayjs.sp
    axios.sp
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
