const vscode = require("vscode");
const cp = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

function parseDiagnostics(output) {
  const diagnostics = [];
  const seen = new Set();
  const pattern = /spearc (error|warning|\uC624\uB958|\uACBD\uACE0) \[line (\d+):(\d+)\] (.+)/g;
  let match;

  while ((match = pattern.exec(output)) !== null) {
    const severity = match[1] === "warning" || match[1] === "\uACBD\uACE0"
      ? vscode.DiagnosticSeverity.Warning
      : vscode.DiagnosticSeverity.Error;
    const line = Math.max(Number(match[2]) - 1, 0);
    const col = Math.max(Number(match[3]) - 1, 0);
    const message = match[4];
    const key = `${severity}:${line}:${col}:${message}`;
    if (seen.has(key)) {
      continue;
    }
    seen.add(key);
    const diagnostic = new vscode.Diagnostic(
      new vscode.Range(line, col, line, col + 1),
      message,
      severity
    );
    diagnostic.source = "spearc";

    const lower = message.toLowerCase();
    if (lower.includes("unused import")) {
      diagnostic.code = "import.unused";
      diagnostic.tags = [vscode.DiagnosticTag.Unnecessary];
    } else if (lower.includes("duplicate import")) {
      diagnostic.code = "import.duplicate";
      diagnostic.tags = [vscode.DiagnosticTag.Unnecessary];
    } else if (lower.includes("unused variable") || lower.includes("unused parameter")) {
      diagnostic.code = "symbol.unused";
      diagnostic.tags = [vscode.DiagnosticTag.Unnecessary];
    } else if (lower.includes("unreachable code")) {
      diagnostic.code = "flow.unreachable";
      diagnostic.tags = [vscode.DiagnosticTag.Unnecessary];
    } else if (lower.includes("condition is always")) {
      diagnostic.code = "flow.constant-condition";
    } else if (
      lower.includes("expected ';'") ||
      lower.includes("expected ')'") ||
      lower.includes("expected '('") ||
      lower.includes("expected '}'") ||
      lower.includes("expected '{'") ||
      lower.startsWith("unexpected ")
    ) {
      diagnostic.code = "syntax";
    } else if (
      lower.includes("unknown variable") ||
      lower.includes("unknown function") ||
      lower.includes("does not return") ||
      lower.includes("type mismatch") ||
      lower.includes("expected numeric expression") ||
      lower.includes("expected text expression") ||
      lower.includes("expected list expression")
    ) {
      diagnostic.code = "type";
    } else if (
      lower.includes("break is only valid") ||
      lower.includes("continue is only valid")
    ) {
      diagnostic.code = "flow.control";
    }

    diagnostics.push(diagnostic);
  }

  return diagnostics;
}

function compilerPath(document) {
  const configured = vscode.workspace.getConfiguration("spear").get("compilerPath", "").trim();
  if (configured) {
    return configured;
  }

  const folder = vscode.workspace.getWorkspaceFolder(document.uri);
  if (folder) {
    const workspaceCompiler = path.join(folder.uri.fsPath, "build", "spearc.exe");
    if (fs.existsSync(workspaceCompiler)) {
      return workspaceCompiler;
    }
  }

  const localAppData = process.env.LOCALAPPDATA || "";
  if (localAppData) {
    const installedCompiler = path.join(localAppData, "Programs", "Spear", "bin", "spearc.exe");
    if (fs.existsSync(installedCompiler)) {
      return installedCompiler;
    }
  }

  try {
    const where = cp.spawnSync("where", ["spearc.exe"], { encoding: "utf8" });
    if (where.status === 0) {
      const first = String(where.stdout || "").split(/\r?\n/).map((line) => line.trim()).find(Boolean);
      if (first && fs.existsSync(first)) {
        return first;
      }
    }
  } catch {}

  return null;
}

async function validateDocument(document, collection) {
  if (document.languageId !== "spear" || document.uri.scheme !== "file") {
    return;
  }

  const spearc = compilerPath(document);
  if (!spearc || !fs.existsSync(spearc)) {
    collection.delete(document.uri);
    return;
  }

  const originalPath = document.fileName;
  await new Promise((resolve) => {
    const child = cp.spawn(spearc, ["--check-stdin", originalPath], {
      cwd: path.dirname(originalPath),
      stdio: ["pipe", "pipe", "pipe"]
    });

    let stdout = "";
    let stderr = "";

    child.stdout.setEncoding("utf8");
    child.stderr.setEncoding("utf8");
    child.stdout.on("data", (chunk) => {
      stdout += chunk;
    });
    child.stderr.on("data", (chunk) => {
      stderr += chunk;
    });
    child.on("error", (error) => {
      collection.set(document.uri, [
        new vscode.Diagnostic(
          new vscode.Range(0, 0, 0, 1),
          String(error?.message || error),
          vscode.DiagnosticSeverity.Error
        )
      ]);
      resolve();
    });
    child.on("close", (code) => {
      const output = `${stderr}\n${stdout}`;
      const diagnosticsList = parseDiagnostics(output);
      if (code === 0) {
        collection.set(document.uri, diagnosticsList);
        resolve();
        return;
      }
      if (diagnosticsList.length > 0) {
        collection.set(document.uri, diagnosticsList);
      } else {
        collection.set(document.uri, [
          new vscode.Diagnostic(
            new vscode.Range(0, 0, 0, 1),
            output.trim() || "Spear check failed",
            vscode.DiagnosticSeverity.Error
          )
        ]);
      }
      resolve();
    });

    child.stdin.end(document.getText());
  });
}

function activate(context) {
  const diagnostics = vscode.languages.createDiagnosticCollection("spear");
  const pending = new Map();

  function schedule(document) {
    const key = document.uri.toString();
    const timer = pending.get(key);
    if (timer) {
      clearTimeout(timer);
    }
    pending.set(
      key,
      setTimeout(() => {
        pending.delete(key);
        validateDocument(document, diagnostics);
      }, 180)
    );
  }

  context.subscriptions.push(diagnostics);
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument(schedule),
    vscode.workspace.onDidChangeTextDocument((event) => schedule(event.document)),
    vscode.workspace.onDidSaveTextDocument(schedule),
    vscode.workspace.onDidCloseTextDocument((document) => diagnostics.delete(document.uri))
  );

  for (const document of vscode.workspace.textDocuments) {
    schedule(document);
  }
}

function deactivate() {}

module.exports = { activate, deactivate };
