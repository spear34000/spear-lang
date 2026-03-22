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

    let manifest = {
        let sharp = full_input.join("sharp.toml");
        if sharp.is_file() {
            sharp
        } else {
            full_input.join("spear.toml")
        }
    };

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
