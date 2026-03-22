use sharp_common::{
    ensure_dir, exe_dir, load_lang_from_dir, normalize_windows_path, parse_manifest_array, project_name,
    render_starter_main, render_starter_manifest, resolve_bundled_gcc, resolve_manifest_path,
    resolve_project_source, resolve_tool, upsert_manifest_array, Lang,
};
use std::env;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};
use std::process::{Command, ExitCode, Stdio};
use std::thread;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

fn text(lang: Lang, key: &str) -> String {
    match key {
        "error_prefix" => match lang {
            Lang::Ko => "sharp 오류".to_string(),
            Lang::En => "sharp error".to_string(),
        },
        "usage" => match lang {
            Lang::Ko => "사용법:\n  sharp\n  sharp file.sp\n  sharp <folder>\n  sharp build [file.sp|folder]\n  sharp serve [file.sp|folder]\n  sharp check [file.sp|folder]\n  sharp new <name>\n  sharp add pip <package> [folder]\n  sharp add npm <package> [folder]\n".to_string(),
            Lang::En => "usage:\n  sharp\n  sharp file.sp\n  sharp <folder>\n  sharp build [file.sp|folder]\n  sharp serve [file.sp|folder]\n  sharp check [file.sp|folder]\n  sharp new <name>\n  sharp add pip <package> [folder]\n  sharp add npm <package> [folder]\n".to_string(),
        },
        "created" => match lang {
            Lang::Ko => "프로젝트 생성 완료".to_string(),
            Lang::En => "created project".to_string(),
        },
        "checked" => match lang {
            Lang::Ko => "검사 완료".to_string(),
            Lang::En => "checked".to_string(),
        },
        "built" => match lang {
            Lang::Ko => "빌드 완료".to_string(),
            Lang::En => "built".to_string(),
        },
        "compile_failed" => match lang {
            Lang::Ko => "sharp 컴파일 오류: 소스 컴파일에 실패했습니다".to_string(),
            Lang::En => "sharp compile error: source compilation failed".to_string(),
        },
        "backend_failed" => match lang {
            Lang::Ko => "sharp 백엔드 오류: 네이티브 빌드에 실패했습니다".to_string(),
            Lang::En => "sharp backend error: native build failed".to_string(),
        },
        "details" => match lang {
            Lang::Ko => "자세한 내용".to_string(),
            Lang::En => "details".to_string(),
        },
        "serve_prefix" => "sharp serve".to_string(),
        "added" => match lang {
            Lang::Ko => "의존성 추가 완료".to_string(),
            Lang::En => "added dependency".to_string(),
        },
        "install_warn" => match lang {
            Lang::Ko => "도구를 찾지 못해 manifest만 갱신했습니다".to_string(),
            Lang::En => "updated manifest only because the package tool was not available".to_string(),
        },
        _ => key.to_string(),
    }
}

fn fail(lang: Lang, message: &str) -> ! {
    eprintln!("{}: {}", text(lang, "error_prefix"), message);
    std::process::exit(1);
}

fn write_file(path: &Path, content: &str) -> io::Result<()> {
    if let Some(parent) = path.parent() {
        ensure_dir(parent)?;
    }
    fs::write(path, content)
}

fn create_project(lang: Lang, name: &str) -> io::Result<()> {
    let root = PathBuf::from(name);
    if root.exists() {
        fail(lang, "target folder already exists");
    }
    ensure_dir(&root)?;
    write_file(&root.join("sharp.toml"), &render_starter_manifest(&project_name(&root)))?;
    write_file(&root.join("main.sp"), &render_starter_main(&project_name(&root)))?;
    write_file(&root.join(".gitignore"), "build/\n")?;
    println!(
        "{} {}",
        text(lang, "created"),
        normalize_windows_path(root.canonicalize()?).display()
    );
    Ok(())
}

fn temp_runtime_dir() -> io::Result<PathBuf> {
    let mut path = env::temp_dir();
    let pid = std::process::id();
    let stamp = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_millis();
    path.push(format!("sharp-{}-{}", pid, stamp));
    ensure_dir(&path)?;
    Ok(path)
}

fn run_capture(mut cmd: Command, log_path: &Path) -> io::Result<i32> {
    let output = cmd.output()?;
    let mut log = Vec::new();
    log.extend_from_slice(&output.stdout);
    log.extend_from_slice(&output.stderr);
    fs::write(log_path, log)?;
    Ok(output.status.code().unwrap_or(1))
}

fn run_console(mut cmd: Command) -> io::Result<i32> {
    let status = cmd.status()?;
    Ok(status.code().unwrap_or(1))
}

fn run_quiet(mut cmd: Command) -> io::Result<i32> {
    let status = cmd.stdout(Stdio::null()).stderr(Stdio::null()).status()?;
    Ok(status.code().unwrap_or(1))
}

fn run_status(mut cmd: Command) -> io::Result<i32> {
    let status = cmd.status()?;
    Ok(status.code().unwrap_or(1))
}

fn print_log(prefix: &str, log_path: &Path) {
    eprintln!("{}", prefix);
    match fs::read_to_string(log_path) {
        Ok(body) => eprintln!("{}", body),
        Err(_) => eprintln!("details were not available"),
    }
}

fn resolve_project_dir(raw: Option<&str>) -> io::Result<PathBuf> {
    let input = raw.unwrap_or(".");
    let full = normalize_windows_path(fs::canonicalize(input)?);
    if full.is_dir() {
        return Ok(full);
    }
    if let Some(parent) = full.parent() {
        return Ok(parent.to_path_buf());
    }
    Err(io::Error::new(io::ErrorKind::NotFound, "could not resolve project folder"))
}

fn add_dependency(lang: Lang, ecosystem: &str, package: &str, raw_root: Option<&str>) -> io::Result<()> {
    let project_root = resolve_project_dir(raw_root)?;
    let manifest = resolve_manifest_path(&project_root);
    let vendor_root = project_root.join(".sharp").join("vendor");
    let status = if ecosystem.eq_ignore_ascii_case("pip") {
        let py_root = vendor_root.join("python");
        ensure_dir(&py_root)?;
        upsert_manifest_array(&manifest, "pip", package)?;
        let direct = run_status({
            let mut cmd = Command::new("python");
            cmd.arg("-m")
                .arg("pip")
                .arg("install")
                .arg("--target")
                .arg(&py_root)
                .arg(package)
                .current_dir(&project_root);
            cmd
        });
        match direct {
            Ok(code) if code == 0 => Ok(0),
            _ => run_status({
                let mut cmd = Command::new("py");
                cmd.arg("-3")
                    .arg("-m")
                    .arg("pip")
                    .arg("install")
                    .arg("--target")
                    .arg(&py_root)
                    .arg(package)
                    .current_dir(&project_root);
                cmd
            }),
        }
    } else {
        let node_root = vendor_root.join("node");
        ensure_dir(&node_root)?;
        upsert_manifest_array(&manifest, "npm", package)?;
        run_status({
            let mut cmd = Command::new("cmd");
            cmd.arg("/c")
                .arg("npm")
                .arg("install")
                .arg("--prefix")
                .arg(&node_root)
                .arg(package)
                .current_dir(&project_root);
            cmd
        })
    };
    println!("{} {}", text(lang, "added"), package);
    println!("manifest: {}", manifest.display());
    match status {
        Ok(0) => {}
        _ => println!("{}", text(lang, "install_warn")),
    }
    Ok(())
}

fn apply_interop_env(cmd: &mut Command, project_root: &Path) {
    let manifest = resolve_manifest_path(project_root);
    let pip = parse_manifest_array(&manifest, "pip");
    if !pip.is_empty() {
        let py_vendor = project_root.join(".sharp").join("vendor").join("python");
        if py_vendor.is_dir() {
            cmd.env("SHARP_PYTHONPATH", py_vendor);
        }
    }
    let npm = parse_manifest_array(&manifest, "npm");
    if !npm.is_empty() {
        let node_modules = project_root.join(".sharp").join("vendor").join("node").join("node_modules");
        if node_modules.is_dir() {
            cmd.env("SHARP_NODE_PATH", node_modules);
        }
    }
}

fn main() -> ExitCode {
    let bin_dir = match exe_dir() {
        Ok(v) => v,
        Err(err) => {
            eprintln!("sharp error: {}", err);
            return ExitCode::from(1);
        }
    };
    let lang = load_lang_from_dir(&bin_dir);
    let args: Vec<String> = env::args().collect();

    if args.len() >= 2 && args[1].eq_ignore_ascii_case("new") {
        if args.len() < 3 {
            fail(lang, "expected a project name");
        }
        if let Err(err) = create_project(lang, &args[2]) {
            fail(lang, &err.to_string());
        }
        return ExitCode::SUCCESS;
    }
    if args.len() >= 4 && args[1].eq_ignore_ascii_case("add") {
        let ecosystem = &args[2];
        if !ecosystem.eq_ignore_ascii_case("pip") && !ecosystem.eq_ignore_ascii_case("npm") {
            fail(lang, "expected 'pip' or 'npm' after 'add'");
        }
        if let Err(err) = add_dependency(lang, ecosystem, &args[3], args.get(4).map(String::as_str)) {
            fail(lang, &err.to_string());
        }
        return ExitCode::SUCCESS;
    }

    let mut mode = "run";
    let mut raw_input = ".";
    if args.len() >= 2 {
        match args[1].to_ascii_lowercase().as_str() {
            "build" | "serve" | "check" => {
                mode = &args[1];
                raw_input = args.get(2).map(String::as_str).unwrap_or(".");
            }
            "service" => {
                mode = "serve";
                raw_input = args.get(2).map(String::as_str).unwrap_or(".");
            }
            _ => raw_input = &args[1],
        }
    }

    let (input, project_root, stem) = match resolve_project_source(Some(raw_input)) {
        Ok(v) => v,
        Err(err) => fail(lang, &err.to_string()),
    };

    let sharpc = match resolve_tool(&bin_dir, &["sharpc.exe", "spearc.exe"]) {
        Some(v) => v,
        None => fail(lang, "installed sharpc.exe was not found next to sharp.exe"),
    };

    let build_dir = project_root.join("build");
    if let Err(err) = ensure_dir(&build_dir) {
        fail(lang, &err.to_string());
    }

    let temp_dir = if mode == "build" {
        None
    } else {
        Some(temp_runtime_dir().unwrap_or_else(|err| fail(lang, &err.to_string())))
    };
    let runtime_dir = temp_dir.as_ref().unwrap_or(&build_dir);
    let c_out = runtime_dir.join(format!("{}.c", stem));
    let exe_out = runtime_dir.join(format!("{}.exe", stem));
    let front_log = runtime_dir.join(format!("{}.sharpc.log", stem));
    let back_log = runtime_dir.join(format!("{}.native.log", stem));

    let compile_exit = if mode == "check" {
        run_capture(
            {
                let mut cmd = Command::new(&sharpc);
                cmd.arg("--check").arg(&input).current_dir(&project_root);
                apply_interop_env(&mut cmd, &project_root);
                cmd
            },
            &front_log,
        )
    } else {
        run_capture(
            {
                let mut cmd = Command::new(&sharpc);
                cmd.arg(&input).arg("-o").arg(&c_out).current_dir(&project_root);
                apply_interop_env(&mut cmd, &project_root);
                cmd
            },
            &front_log,
        )
    };
    let compile_exit = compile_exit.unwrap_or(1);
    if compile_exit != 0 {
        if mode == "build" {
            eprintln!("{}", text(lang, "compile_failed"));
            eprintln!("{}: {}", text(lang, "details"), front_log.display());
        } else {
            print_log(&text(lang, "compile_failed"), &front_log);
        }
        return ExitCode::from(1);
    }

    if mode == "check" {
        if let Some(temp) = temp_dir {
            let _ = fs::remove_dir_all(temp);
        }
        println!("{} {}", text(lang, "checked"), input.display());
        return ExitCode::SUCCESS;
    }

    let gcc = match resolve_bundled_gcc(&bin_dir) {
        Some(v) => v,
        None => fail(lang, "gcc.exe was not found. Re-run setup and install the bundled toolchain."),
    };

    let native_exit = run_capture(
        {
            let mut cmd = Command::new(gcc);
            cmd.arg("-O3")
                .arg("-w")
                .arg("-std=c11")
                .arg("-o")
                .arg(&exe_out)
                .arg(&c_out)
                .current_dir(&project_root);
            apply_interop_env(&mut cmd, &project_root);
            cmd
        },
        &back_log,
    )
    .unwrap_or(1);
    if native_exit != 0 {
        if mode == "build" {
            eprintln!("{}", text(lang, "backend_failed"));
            eprintln!("{}: {}", text(lang, "details"), back_log.display());
        } else {
            print_log(&text(lang, "backend_failed"), &back_log);
        }
        return ExitCode::from(1);
    }

    let _ = fs::remove_file(&front_log);
    let _ = fs::remove_file(&back_log);
    if mode != "build" {
        let _ = fs::remove_file(&c_out);
    }

    if mode == "build" {
        println!("{} {}", text(lang, "built"), exe_out.display());
        return ExitCode::SUCCESS;
    }

    let run_exit = if mode == "serve" {
        run_quiet({
            let mut cmd = Command::new(&exe_out);
            cmd.current_dir(&project_root);
            apply_interop_env(&mut cmd, &project_root);
            cmd
        })
    } else {
        run_console({
            let mut cmd = Command::new(&exe_out);
            cmd.current_dir(&project_root);
            apply_interop_env(&mut cmd, &project_root);
            cmd
        })
    }
    .unwrap_or(1);

    if mode == "serve" && run_exit == 0 {
        let html = if build_dir.join("sharp-ui.html").is_file() {
            "http://127.0.0.1:4173/sharp-ui.html"
        } else if build_dir.join("spear-ui.html").is_file() {
            "http://127.0.0.1:4173/spear-ui.html"
        } else {
            "http://127.0.0.1:4173/"
        };
        println!("{}: {}", text(lang, "serve_prefix"), html);
        let serve_script = if bin_dir.join("runtime").join("serve_static.ps1").is_file() {
            bin_dir.join("runtime").join("serve_static.ps1")
        } else {
            bin_dir.parent().unwrap_or(&bin_dir).join("runtime").join("serve_static.ps1")
        };
        let _ = run_console({
            let mut cmd = Command::new("cmd");
            cmd.arg("/c")
                .arg("start")
                .arg("")
                .arg("cmd")
                .arg("/c")
                .arg(format!("ping -n 3 127.0.0.1 >nul && start \"\" \"{}\"", html));
            cmd
        });
        let _ = thread::sleep(Duration::from_millis(150));
        let _ = run_console({
            let mut cmd = Command::new("powershell");
            cmd.arg("-NoProfile")
                .arg("-ExecutionPolicy")
                .arg("Bypass")
                .arg("-File")
                .arg(serve_script)
                .arg("-Root")
                .arg(&build_dir)
                .arg("-Port")
                .arg("4173")
                .current_dir(&project_root);
            apply_interop_env(&mut cmd, &project_root);
            cmd
        });
    }

    if let Some(temp) = temp_dir {
        let _ = fs::remove_file(&exe_out);
        let _ = fs::remove_dir_all(temp);
    }
    if run_exit == 0 {
        ExitCode::SUCCESS
    } else {
        ExitCode::from(1)
    }
}
