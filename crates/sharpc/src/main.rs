use sharp_common::{exe_dir, load_lang_from_dir, resolve_runtime_tool, Lang};
use std::env;
use std::path::PathBuf;
use std::process::{Command, ExitCode};

fn text(lang: Lang, key: &str) -> String {
    match key {
        "error_prefix" => match lang {
            Lang::Ko => "sharpc ?ㅻ쪟".to_string(),
            Lang::En => "sharpc error".to_string(),
        },
        "usage" => match lang {
            Lang::Ko => "?ъ슜踰?\n  sharpc <source.sharp> [-o output.c]\n  sharpc --check <source.sharp>\n  sharpc --check-stdin <virtual-file.sharp>\n".to_string(),
            Lang::En => "usage:\n  sharpc <source.sharp> [-o output.c]\n  sharpc --check <source.sharp>\n  sharpc --check-stdin <virtual-file.sharp>\n".to_string(),
        },
        "missing_backend" => "legacy compiler backend (sharpc-c.exe) was not found".to_string(),
        _ => key.to_string(),
    }
}

fn fail(lang: Lang, message: &str) -> ! {
    eprintln!("{}: {}", text(lang, "error_prefix"), message);
    std::process::exit(1);
}

fn resolve_backend(bin_dir: &PathBuf) -> Option<PathBuf> {
    resolve_runtime_tool(
        bin_dir,
        &["sharpc-c.exe", "spearc-c.exe", "sharpc-legacy.exe", "spearc-legacy.exe"],
    )
}

fn main() -> ExitCode {
    let bin_dir = match exe_dir() {
        Ok(v) => v,
        Err(err) => {
            eprintln!("sharpc error: {}", err);
            return ExitCode::from(1);
        }
    };
    let lang = load_lang_from_dir(&bin_dir);
    let args: Vec<String> = env::args().skip(1).collect();
    if args.is_empty() {
        eprint!("{}", text(lang, "usage"));
        return ExitCode::from(1);
    }
    let backend = match resolve_backend(&bin_dir) {
        Some(v) => v,
        None => fail(lang, &text(lang, "missing_backend")),
    };
    match Command::new(backend).args(args).status() {
        Ok(status) => ExitCode::from(status.code().unwrap_or(1) as u8),
        Err(err) => fail(lang, &err.to_string()),
    }
}

