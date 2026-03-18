const vscode = require("vscode");
const cp = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

function parseDiagnostic(output) {
  const match = output.match(/spearc error \[line (\d+):(\d+)\] (.+)/);
  if (!match) {
    return null;
  }
  const line = Math.max(Number(match[1]) - 1, 0);
  const col = Math.max(Number(match[2]) - 1, 0);
  const message = match[3];
  return new vscode.Diagnostic(
    new vscode.Range(line, col, line, col + 1),
    message,
    vscode.DiagnosticSeverity.Error
  );
}

function compilerPath(document) {
  const configured = vscode.workspace.getConfiguration("spear").get("compilerPath", "").trim();
  if (configured) {
    return configured;
  }

  const folder = vscode.workspace.getWorkspaceFolder(document.uri);
  if (!folder) {
    return null;
  }
  return path.join(folder.uri.fsPath, "build", "spearc.exe");
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
      if (code === 0) {
        collection.set(document.uri, []);
        resolve();
        return;
      }
      const output = `${stderr}\n${stdout}`;
      const diagnostic = parseDiagnostic(output);
      if (diagnostic) {
        collection.set(document.uri, [diagnostic]);
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
