use std::env;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Lang {
    En,
    Ko,
}

pub fn load_lang_from_dir(dir: &Path) -> Lang {
    for base in [Some(dir), dir.parent()] {
        let Some(base_dir) = base else { continue };
        for file_name in ["sharp-lang.txt", "spear-lang.txt"] {
            let path = base_dir.join(file_name);
            if let Ok(raw) = fs::read_to_string(path) {
                let trimmed = raw.trim_start_matches('\u{feff}').trim();
                if trimmed.eq_ignore_ascii_case("ko") {
                    return Lang::Ko;
                }
                if trimmed.eq_ignore_ascii_case("en") {
                    return Lang::En;
                }
            }
        }
    }
    Lang::En
}

pub fn exe_dir() -> io::Result<PathBuf> {
    let exe = env::current_exe()?;
    exe.parent()
        .map(Path::to_path_buf)
        .ok_or_else(|| io::Error::new(io::ErrorKind::Other, "cannot resolve executable directory"))
}

pub fn ensure_dir(path: &Path) -> io::Result<()> {
    fs::create_dir_all(path)
}

pub fn normalize_windows_path(path: PathBuf) -> PathBuf {
    let raw = path.to_string_lossy();
    if let Some(rest) = raw.strip_prefix(r"\\?\") {
        return PathBuf::from(rest);
    }
    path
}

pub fn file_stem(path: &Path) -> String {
    path.file_stem()
        .and_then(|v| v.to_str())
        .unwrap_or("app")
        .to_string()
}

pub fn project_name(path: &Path) -> String {
    path.file_name()
        .and_then(|v| v.to_str())
        .unwrap_or("app")
        .to_string()
}

pub fn parse_manifest_value(manifest_path: &Path, key: &str) -> Option<String> {
    let body = fs::read_to_string(manifest_path).ok()?;
    for line in body.lines() {
        let trimmed = line.trim();
        if !trimmed.starts_with(key) {
            continue;
        }
        let rest = trimmed[key.len()..].trim_start();
        let value = rest.strip_prefix('=')?.trim_start();
        let value = value.strip_prefix('"')?;
        let end = value.find('"')?;
        return Some(value[..end].to_string());
    }
    None
}

pub fn parse_manifest_array(manifest_path: &Path, key: &str) -> Vec<String> {
    let body = match fs::read_to_string(manifest_path) {
        Ok(v) => v,
        Err(_) => return Vec::new(),
    };
    for line in body.lines() {
        let trimmed = line.trim();
        if !trimmed.starts_with(key) {
            continue;
        }
        let rest = trimmed[key.len()..].trim_start();
        let Some(value) = rest.strip_prefix('=') else { continue };
        let value = value.trim();
        let Some(inner) = value.strip_prefix('[').and_then(|v| v.strip_suffix(']')) else { continue };
        let mut out = Vec::new();
        for part in inner.split(',') {
            let item = part.trim().trim_matches('"').trim();
            if !item.is_empty() {
                out.push(item.to_string());
            }
        }
        return out;
    }
    Vec::new()
}

pub fn resolve_manifest_path(project_dir: &Path) -> PathBuf {
    let sharp = project_dir.join("sharp.toml");
    if sharp.is_file() {
        return sharp;
    }
    let legacy = project_dir.join("spear.toml");
    if legacy.is_file() {
        return legacy;
    }
    sharp
}

pub fn render_manifest_array(key: &str, values: &[String]) -> String {
    let parts: Vec<String> = values.iter().map(|v| format!("\"{}\"", v)).collect();
    format!("{key} = [{}]", parts.join(", "))
}

pub fn upsert_manifest_array(manifest_path: &Path, key: &str, value: &str) -> io::Result<()> {
    let mut lines: Vec<String> = if manifest_path.is_file() {
        fs::read_to_string(manifest_path)?
            .lines()
            .map(|v| v.to_string())
            .collect()
    } else {
        vec![
            format!("name = \"{}\"", project_name(manifest_path.parent().unwrap_or(Path::new(".")))),
            "entry = \"main.sp\"".to_string(),
            "kind = \"app\"".to_string(),
        ]
    };
    let mut values = parse_manifest_array(manifest_path, key);
    if !values.iter().any(|v| v.eq_ignore_ascii_case(value)) {
        values.push(value.to_string());
    }
    let rendered = render_manifest_array(key, &values);
    let prefix = format!("{key} =");
    lines.retain(|line| !line.trim().starts_with(&prefix));
    lines.push(rendered);
    fs::write(manifest_path, format!("{}\n", lines.join("\n")))
}

pub fn sanitize_module_name(package: &str) -> String {
    let mut out = String::new();
    for ch in package.chars() {
        if ch.is_ascii_alphanumeric() {
            out.push(ch.to_ascii_lowercase());
        } else {
            out.push('_');
        }
    }
    while out.contains("__") {
        out = out.replace("__", "_");
    }
    out.trim_matches('_').to_string()
}

pub fn interop_python_module_name(package: &str) -> String {
    format!("{}_sharp", sanitize_module_name(package))
}

pub fn interop_node_shim_name(package: &str) -> String {
    format!("{}_sharp.cjs", sanitize_module_name(package))
}

pub fn render_interop_example(package: &str, module_name: &str) -> String {
    if package.eq_ignore_ascii_case("requests") {
        return format!(
            "import \"interop/{module_name}.sp\";\n\nrun {{\n    print(status_code(\"https://example.com\"));\n}}\n"
        );
    }
    if package.eq_ignore_ascii_case("dayjs") {
        return format!(
            "import \"interop/{module_name}.sp\";\n\nrun {{\n    print(format_now(\"YYYY-MM-DD\"));\n}}\n"
        );
    }
    if package.eq_ignore_ascii_case("axios") {
        return format!(
            "import \"interop/{module_name}.sp\";\n\nrun {{\n    print(status_code(\"https://example.com\"));\n}}\n"
        );
    }
    if package.eq_ignore_ascii_case("numpy") {
        return format!(
            "import \"interop/{module_name}.sp\";\n\nrun {{\n    print(array_sum(\"[1,2,3]\"));\n    print(mean(\"[1,2,3]\"));\n}}\n"
        );
    }
    if package.eq_ignore_ascii_case("pandas") {
        return format!(
            "import \"interop/{module_name}.sp\";\n\nrun {{\n    print(read_csv_head(\"data.csv\", 5));\n}}\n"
        );
    }
    format!(
        "import \"interop/{module_name}.sp\";\n\nrun {{\n    print(\"interop ready\");\n}}\n"
    )
}

pub fn render_interop_wrapper(ecosystem: &str, package: &str, module_name: &str, target: &str) -> String {
    let imports = if ecosystem.eq_ignore_ascii_case("pip")
        && (package.eq_ignore_ascii_case("requests")
            || package.eq_ignore_ascii_case("numpy")
            || package.eq_ignore_ascii_case("pandas")
            || package.eq_ignore_ascii_case("demo_python"))
        || ecosystem.eq_ignore_ascii_case("npm")
            && (package.eq_ignore_ascii_case("dayjs")
                || package.eq_ignore_ascii_case("axios")
                || package.eq_ignore_ascii_case("./demo_node.cjs"))
    {
        "import \"std/interop.sp\";\nimport \"std/json.sp\";"
    } else {
        "import \"std/interop.sp\";"
    };
    let handle_fn = if ecosystem.eq_ignore_ascii_case("pip") {
        format!("    return pip_module(\"{}\");", target)
    } else {
        format!("    return npm_module(\"{}\");", target)
    };
    let call_fn = if ecosystem.eq_ignore_ascii_case("pip") {
        "    return py_call(module_handle(), fn_name, payload);"
    } else {
        "    return js_call(module_handle(), fn_name, payload);"
    };
    let call0_fn = if ecosystem.eq_ignore_ascii_case("pip") {
        "    return py_empty(module_handle(), fn_name);"
    } else {
        "    return js_empty(module_handle(), fn_name);"
    };
    let preset = if ecosystem.eq_ignore_ascii_case("pip") && package.eq_ignore_ascii_case("requests") {
        "function text get_text(text url) {\n    return call(\"get_text\", json_object1(json_field(\"url\", json_text(url))));\n}\n\nfunction text get_json(text url) {\n    return call(\"get_json\", json_object1(json_field(\"url\", json_text(url))));\n}\n\nfunction text post_json(text url, text body_json) {\n    return call(\"post_json\", json_object2(json_field(\"url\", json_text(url)), json_field(\"body\", json_parse(body_json))));\n}\n\nfunction number status_code(text url) {\n    return num(call(\"status_code\", json_object1(json_field(\"url\", json_text(url)))));\n}\n"
    } else if ecosystem.eq_ignore_ascii_case("pip") && package.eq_ignore_ascii_case("numpy") {
        "function number array_sum(text numbers_json) {\n    return num(call(\"array_sum\", json_object1(json_field(\"values\", json_parse(numbers_json)))));\n}\n\nfunction number mean(text numbers_json) {\n    return num(call(\"mean\", json_object1(json_field(\"values\", json_parse(numbers_json)))));\n}\n"
    } else if ecosystem.eq_ignore_ascii_case("pip") && package.eq_ignore_ascii_case("pandas") {
        "function text read_csv_head(text path, number rows) {\n    return call(\"read_csv_head\", json_object2(json_field(\"path\", json_text(path)), json_field(\"rows\", json_number(rows))));\n}\n\nfunction text columns(text path) {\n    return call(\"columns\", json_object1(json_field(\"path\", json_text(path))));\n}\n"
    } else if ecosystem.eq_ignore_ascii_case("npm") && package.eq_ignore_ascii_case("dayjs") {
        "function text format_now(text pattern) {\n    return call(\"format_now\", json_object1(json_field(\"pattern\", json_text(pattern))));\n}\n\nfunction text add_days(text iso_value, number days, text pattern) {\n    return call(\"add_days\", json_object3(json_field(\"value\", json_text(iso_value)), json_field(\"days\", json_number(days)), json_field(\"pattern\", json_text(pattern))));\n}\n\nfunction text from_iso(text iso_value, text pattern) {\n    return call(\"from_iso\", json_object2(json_field(\"value\", json_text(iso_value)), json_field(\"pattern\", json_text(pattern))));\n}\n"
    } else if ecosystem.eq_ignore_ascii_case("npm") && package.eq_ignore_ascii_case("axios") {
        "function text get_text(text url) {\n    return call(\"get_text\", json_object1(json_field(\"url\", json_text(url))));\n}\n\nfunction text get_json(text url) {\n    return call(\"get_json\", json_object1(json_field(\"url\", json_text(url))));\n}\n\nfunction text post_json(text url, text body_json) {\n    return call(\"post_json\", json_object2(json_field(\"url\", json_text(url)), json_field(\"body\", json_parse(body_json))));\n}\n\nfunction number status_code(text url) {\n    return num(call(\"status_code\", json_object1(json_field(\"url\", json_text(url)))));\n}\n"
    } else if ecosystem.eq_ignore_ascii_case("pip") && package.eq_ignore_ascii_case("demo_python") {
        "function text render(text name) {\n    return call(\"render\", json_object1(json_field(\"name\", json_text(name))));\n}\n"
    } else if ecosystem.eq_ignore_ascii_case("npm") && package.eq_ignore_ascii_case("./demo_node.cjs") {
        "function text render(text name) {\n    return call(\"render\", json_object1(json_field(\"name\", json_text(name))));\n}\n"
    } else {
        ""
    };
    format!(
        "{imports}\n\npackage app;\nmodule {module_name};\n\nfunction text module_handle() {{\n{handle_fn}\n}}\n\nfunction text call(text fn_name, text payload) {{\n{call_fn}\n}}\n\nfunction text call0(text fn_name) {{\n{call0_fn}\n}}\n\n{preset}"
    )
}

pub fn render_python_shim(package: &str) -> Option<String> {
    if package.eq_ignore_ascii_case("requests") {
        return Some(
            "import json\nimport requests\n\n\ndef get_text(payload):\n    url = payload.get(\"url\", \"\")\n    return requests.get(url, timeout=10).text\n\n\ndef get_json(payload):\n    url = payload.get(\"url\", \"\")\n    return json.dumps(requests.get(url, timeout=10).json())\n\n\ndef post_json(payload):\n    url = payload.get(\"url\", \"\")\n    body = payload.get(\"body\", {})\n    return json.dumps(requests.post(url, json=body, timeout=10).json())\n\n\ndef status_code(payload):\n    url = payload.get(\"url\", \"\")\n    return requests.get(url, timeout=10).status_code\n".to_string(),
        );
    }
    if package.eq_ignore_ascii_case("numpy") {
        return Some(
            "import numpy as np\n\n\ndef array_sum(payload):\n    values = payload.get(\"values\", [])\n    return float(np.sum(np.array(values, dtype=float)))\n\n\ndef mean(payload):\n    values = payload.get(\"values\", [])\n    return float(np.mean(np.array(values, dtype=float)))\n".to_string(),
        );
    }
    if package.eq_ignore_ascii_case("pandas") {
        return Some(
            "import json\nimport pandas as pd\n\n\ndef read_csv_head(payload):\n    path = payload.get(\"path\", \"\")\n    rows = int(payload.get(\"rows\", 5))\n    frame = pd.read_csv(path)\n    return frame.head(rows).to_json(orient=\"records\")\n\n\ndef columns(payload):\n    path = payload.get(\"path\", \"\")\n    frame = pd.read_csv(path)\n    return json.dumps(list(frame.columns))\n".to_string(),
        );
    }
    if package.eq_ignore_ascii_case("demo_python") {
        return Some(
            "import demo_python\n\n\ndef render(payload):\n    return demo_python.render(payload)\n".to_string(),
        );
    }
    None
}

pub fn render_node_shim(package: &str) -> Option<String> {
    if package.eq_ignore_ascii_case("dayjs") {
        return Some(
            "const dayjs = require(\"dayjs\");\n\nexports.format_now = function formatNow(payload) {\n  const pattern = payload?.pattern || \"YYYY-MM-DD\";\n  return dayjs().format(pattern);\n};\n\nexports.add_days = function addDays(payload) {\n  const value = payload?.value || dayjs().toISOString();\n  const days = Number(payload?.days || 0);\n  const pattern = payload?.pattern || \"YYYY-MM-DD\";\n  return dayjs(value).add(days, \"day\").format(pattern);\n};\n\nexports.from_iso = function fromIso(payload) {\n  const value = payload?.value || dayjs().toISOString();\n  const pattern = payload?.pattern || \"YYYY-MM-DD\";\n  return dayjs(value).format(pattern);\n};\n".to_string(),
        );
    }
    if package.eq_ignore_ascii_case("axios") {
        return Some(
            "const axios = require(\"axios\");\n\nexports.get_text = async function getText(payload) {\n  const url = payload?.url || \"\";\n  const response = await axios.get(url, { timeout: 10000, responseType: \"text\", validateStatus: () => true });\n  return typeof response.data === \"string\" ? response.data : JSON.stringify(response.data);\n};\n\nexports.get_json = async function getJson(payload) {\n  const url = payload?.url || \"\";\n  const response = await axios.get(url, { timeout: 10000, validateStatus: () => true });\n  return JSON.stringify(response.data);\n};\n\nexports.post_json = async function postJson(payload) {\n  const url = payload?.url || \"\";\n  const body = payload?.body || {};\n  const response = await axios.post(url, body, { timeout: 10000, validateStatus: () => true });\n  return JSON.stringify(response.data);\n};\n\nexports.status_code = async function statusCode(payload) {\n  const url = payload?.url || \"\";\n  const response = await axios.get(url, { timeout: 10000, validateStatus: () => true });\n  return response.status;\n};\n".to_string(),
        );
    }
    if package.eq_ignore_ascii_case("./demo_node.cjs") {
        return Some(
            "const demo = require(process.cwd() + \"/demo_node.cjs\");\n\nexports.render = function render(payload) {\n  return demo.render(payload);\n};\n".to_string(),
        );
    }
    None
}

pub fn resolve_project_source(raw_input: Option<&str>) -> io::Result<(PathBuf, PathBuf, String)> {
    let input = raw_input.unwrap_or(".");
    let full_input = normalize_windows_path(fs::canonicalize(input)?);

    if full_input.is_file() && full_input.extension().and_then(|v| v.to_str()) == Some("sp") {
        let root = full_input
            .parent()
            .map(Path::to_path_buf)
            .ok_or_else(|| io::Error::new(io::ErrorKind::Other, "cannot resolve project root"))?;
        return Ok((full_input.clone(), root, file_stem(&full_input)));
    }

    if !full_input.is_dir() {
        return Err(io::Error::new(
            io::ErrorKind::NotFound,
            "expected a .sp source file or a Sharp project folder",
        ));
    }

    let manifest = resolve_manifest_path(&full_input);
    if manifest.is_file() {
        if let Some(entry) = parse_manifest_value(&manifest, "entry") {
            let entry_path = full_input.join(entry);
            if entry_path.is_file() {
                let name = parse_manifest_value(&manifest, "name").unwrap_or_else(|| project_name(&full_input));
                return Ok((normalize_windows_path(entry_path), full_input.clone(), name));
            }
        }
    }

    for rel in ["main.sp", "app/main.sp", "src/main.sp"] {
        let candidate = full_input.join(rel);
        if candidate.is_file() {
            return Ok((normalize_windows_path(candidate), full_input.clone(), project_name(&full_input)));
        }
    }

    Err(io::Error::new(
        io::ErrorKind::NotFound,
        "could not find a Sharp project entry file",
    ))
}

pub fn render_starter_manifest(name: &str) -> String {
    format!("name = \"{}\"\nentry = \"main.sp\"\nkind = \"web\"\n", name)
}

pub fn render_starter_main(name: &str) -> String {
    format!(
        "import \"std/web.sp\";\n\nrun {{\n    const title = \"{name}\";\n    string html = landing_page(title, theme_product_bg()) {{\n        centered(box(glass_panel_mod()) {{\n            column_box(gap_space(space_4())) {{\n                markup(\"span\", style_attr(badge_mod())) {{\n                    \"Sharp App\";\n                }};\n                markup(\"h1\", style_attr(hero_title_mod())) {{\n                    \"Build sharp web pages with readable blocks.\";\n                }};\n                markup(\"p\", style_attr(lead_copy_mod())) {{\n                    \"Sharp gives you strong defaults, composable design tokens, and production-friendly page primitives.\";\n                }};\n                row_box(gap_space(space_2())) {{\n                    button_link(button_primary_mod(), \"#start\", \"Start building\");\n                    button_link(button_secondary_mod(), \"#signals\", \"See the layout\");\n                }};\n            }};\n        }});\n    }};\n    write(\"build/sharp-ui.html\", html);\n}}\n"
    )
}

pub fn resolve_bundled_gcc(bin_dir: &Path) -> Option<PathBuf> {
    for root in [Some(bin_dir.to_path_buf()), bin_dir.parent().map(Path::to_path_buf)] {
        let Some(root) = root else { continue };
        let bundled = root.join("toolchain").join("mingw64").join("bin").join("gcc.exe");
        if bundled.is_file() {
            return Some(bundled);
        }
    }
    if let Some(path_var) = env::var_os("PATH") {
        for dir in env::split_paths(&path_var) {
            let candidate = dir.join("gcc.exe");
            if candidate.is_file() {
                return Some(candidate);
            }
        }
    }
    None
}

pub fn resolve_tool(bin_dir: &Path, preferred: &[&str]) -> Option<PathBuf> {
    for name in preferred {
        let candidate = bin_dir.join(name);
        if candidate.is_file() {
            return Some(candidate);
        }
    }
    None
}

pub fn resolve_runtime_tool(bin_dir: &Path, preferred: &[&str]) -> Option<PathBuf> {
    for name in preferred {
        let local = bin_dir.join(name);
        if local.is_file() {
            return Some(local);
        }
        let local_runtime = bin_dir.join("runtime").join(name);
        if local_runtime.is_file() {
            return Some(local_runtime);
        }
        if let Some(root) = bin_dir.parent() {
            let runtime = root.join("runtime").join(name);
            if runtime.is_file() {
                return Some(runtime);
            }
        }
    }
    None
}
