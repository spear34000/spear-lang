# Sharp Interop

Sharp can vendor Python and Node packages into a project and generate wrapper modules for them.

## Add Packages

```bash
sharp add pip requests
sharp add pip matplotlib
sharp add pip plotly
sharp add pip pillow
sharp add pip openai
sharp add pip fastapi
sharp add pip langchain
sharp add pip transformers
sharp add pip discord.py
sharp add npm dayjs
sharp add npm axios
sharp add npm react
sharp add npm three
sharp add npm express
sharp add npm next
sharp add npm tailwindcss
sharp add npm zustand
sharp add npm discord.js
```

When you add a package, Sharp updates `sharp.toml` and creates:

- `.sharp/vendor/...`
- `.sharp/shims/...`
- `interop/<package>.sharp`
- `interop/<package>_example.sharp`
- `std/interop.sharp`
- `std/json.sharp`

## Generated Wrapper Style

Sharp keeps a low-level escape hatch:

```sharp
import "std/interop.sharp";

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

```sharp
import "interop/requests.sharp";

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

```sharp
import "interop/numpy.sharp";

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

```sharp
import "interop/pandas.sharp";

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

```sharp
import "interop/matplotlib.sharp";

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

```sharp
import "interop/plotly.sharp";

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

```sharp
import "interop/pillow.sharp";

run {
    print(image_size("input.png"));
    print(thumbnail("input.png", 320, 240, "thumb.png"));
}
```

Generated helpers:

- `image_size(path)`
- `thumbnail(path, width, height, output_path)`

## OpenAI Preset

After:

```bash
sharp add pip openai
```

you can use:

```sharp
import "interop/openai.sharp";

run {
    print(chat_text("gpt-5.4-mini", "Say hello from Sharp."));
}
```

Generated helpers:

- `chat_text(model, prompt)`

## FastAPI Preset

After:

```bash
sharp add pip fastapi
```

you can use:

```sharp
import "interop/fastapi.sharp";

run {
    print(app_module("SharpApi"));
}
```

Generated helpers:

- `app_module(name)`
- `route_module(name, path)`

## LangChain Preset

After:

```bash
sharp add pip langchain
```

Generated helpers:

- `prompt_template(template)`
- `chain_module(model_name)`

## Transformers Preset

After:

```bash
sharp add pip transformers
```

Generated helpers:

- `sentiment_pipeline(model_name, text_value)`
- `text_generator(model_name)`

## Discord.py Preset

After:

```bash
sharp add pip discord.py
```

Generated helpers:

- `bot_module(name)`
- `command_module(name)`

## Dayjs Preset

After:

```bash
sharp add npm dayjs
```

you can use:

```sharp
import "interop/dayjs.sharp";

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

```sharp
import "interop/axios.sharp";

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

## React Preset

After:

```bash
sharp add npm react
```

you can use:

```sharp
import "interop/react.sharp";

run {
    print(component("HeroCard", "Sharp", "Interop component"));
}
```

Generated helpers:

- `component(name, title, body)`
- `page(name, title, body)`

## Tailwind Preset

After:

```bash
sharp add npm tailwindcss
```

Generated helpers:

- `config_module(content_glob)`
- `utility_block(classes)`

## Zustand Preset

After:

```bash
sharp add npm zustand
```

Generated helpers:

- `store_module(name)`
- `slice_module(name)`

## Express Preset

After:

```bash
sharp add npm express
```

you can use:

```sharp
import "interop/express.sharp";

run {
    print(server_module("sharpServer"));
}
```

Generated helpers:

- `server_module(name)`
- `route_module(name, path)`

## Three Preset

After:

```bash
sharp add npm three
```

you can use:

```sharp
import "interop/three.sharp";

run {
    print(spinning_cube("SharpScene"));
}
```

Generated helpers:

- `scene_module(name)`
- `spinning_cube(name)`

## Discord.js Preset

After:

```bash
sharp add npm discord.js
```

Generated helpers:

- `bot_module(name)`
- `command_module(name)`

## Next Preset

After:

```bash
sharp add npm next
```

you can use:

```sharp
import "interop/next.sharp";

run {
    print(page_module("HomePage", "Sharp", "Next starter from Sharp"));
}
```

Generated helpers:

- `page_module(name, title, body)`
- `api_route(name)`

## Project Layout

Typical project output after adding packages:

```text
my-app/
  sharp.toml
  main.sharp
  interop/
    requests.sharp
    matplotlib.sharp
    plotly.sharp
    pillow.sharp
    openai.sharp
    fastapi.sharp
    langchain.sharp
    transformers.sharp
    discord_py.sharp
    dayjs.sharp
    axios.sharp
    react.sharp
    three.sharp
    express.sharp
    next.sharp
    tailwindcss.sharp
    zustand.sharp
    discord_js.sharp
    requests_example.sharp
    numpy_example.sharp
  std/
    interop.sharp
    json.sharp
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


