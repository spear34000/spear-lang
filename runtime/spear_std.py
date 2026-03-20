from __future__ import annotations

import base64
import csv
import hashlib
import hmac
import json
import math
import os
import random
import secrets
import socket
import sqlite3
import statistics
import subprocess
import time
import urllib.error
import urllib.parse
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

try:
    import tomllib
except ModuleNotFoundError:  # pragma: no cover
    tomllib = None


def _text(value):
    if value is None:
        return ""
    if isinstance(value, str):
        return value
    return json.dumps(value, ensure_ascii=False)


def json_format(payload):
    text = payload.get("text", "")
    indent = int(payload.get("indent", 2))
    data = json.loads(text)
    return json.dumps(data, ensure_ascii=False, indent=indent)


def json_quote(payload):
    return json.dumps(payload if isinstance(payload, str) else _text(payload), ensure_ascii=False)


def json_get(payload):
    text = payload.get("text", "")
    key = payload.get("key", "")
    default = payload.get("default")
    data = json.loads(text)
    value = data.get(key, default) if isinstance(data, dict) else default
    return _text(value)


def json_keys(payload):
    text = payload.get("text", "")
    data = json.loads(text)
    if not isinstance(data, dict):
        return []
    return list(data.keys())


def map_get(payload):
    data = json.loads(payload.get("text", "{}"))
    key = payload.get("key", "")
    default = payload.get("default", "")
    if not isinstance(data, dict):
        return _text(default)
    return _text(data.get(key, default))


def map_set(payload):
    data = json.loads(payload.get("text", "{}"))
    if not isinstance(data, dict):
        data = {}
    key = payload.get("key", "")
    value = payload.get("value")
    data[key] = value
    return json.dumps(data, ensure_ascii=False)


def map_remove(payload):
    data = json.loads(payload.get("text", "{}"))
    if not isinstance(data, dict):
        data = {}
    key = payload.get("key", "")
    data.pop(key, None)
    return json.dumps(data, ensure_ascii=False)


def map_has(payload):
    data = json.loads(payload.get("text", "{}"))
    if not isinstance(data, dict):
        return 0
    return 1 if payload.get("key", "") in data else 0


def result_is_ok(payload):
    data = json.loads(payload.get("text", "{}"))
    return 1 if isinstance(data, dict) and data.get("ok") else 0


def result_value(payload):
    data = json.loads(payload.get("text", "{}"))
    return _text(data.get("value", ""))


def result_error(payload):
    data = json.loads(payload.get("text", "{}"))
    return _text(data.get("error", ""))


def csv_parse(payload):
    text = payload.get("text", "")
    rows = list(csv.reader(text.splitlines()))
    return rows


def csv_stringify(payload):
    rows = payload.get("rows", [])
    from io import StringIO

    buffer = StringIO()
    writer = csv.writer(buffer)
    for row in rows:
        writer.writerow(row)
    return buffer.getvalue()


def fs_read(payload):
    return Path(payload["path"]).read_text(encoding="utf-8")


def fs_write(payload):
    path = Path(payload["path"])
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(payload.get("content", ""), encoding="utf-8")
    return str(path)


def fs_exists(payload):
    return 1 if Path(payload["path"]).exists() else 0


def fs_list(payload):
    path = Path(payload["path"])
    if not path.exists():
        return []
    return [item.name for item in path.iterdir()]


def fs_mkdir(payload):
    Path(payload["path"]).mkdir(parents=True, exist_ok=True)
    return payload["path"]


def fs_remove(payload):
    path = Path(payload["path"])
    if path.is_dir():
        for child in path.iterdir():
            if child.is_file():
                child.unlink()
        path.rmdir()
    elif path.exists():
        path.unlink()
    return payload["path"]


def env_get(payload):
    return os.environ.get(payload.get("name", ""), payload.get("default", ""))


def env_all(_payload):
    return dict(os.environ)


def process_run(payload):
    command = payload.get("command", "")
    cwd = payload.get("cwd") or None
    shell = bool(payload.get("shell", True))
    completed = subprocess.run(
        command,
        cwd=cwd,
        shell=shell,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    return {
        "code": completed.returncode,
        "stdout": completed.stdout,
        "stderr": completed.stderr,
    }


def secure_token(payload):
    size = int(payload.get("bytes", 32))
    if size < 16:
        size = 16
    if size > 64:
        size = 64
    return secrets.token_urlsafe(size)


def secure_compare(payload):
    left = _text(payload.get("left", ""))
    right = _text(payload.get("right", ""))
    return 1 if hmac.compare_digest(left, right) else 0


def pbkdf2_sha256(payload):
    password = payload.get("password", "").encode("utf-8")
    salt = payload.get("salt", "").encode("utf-8")
    rounds = int(payload.get("rounds", 120000))
    if rounds < 100000:
        rounds = 100000
    if rounds > 1000000:
        rounds = 1000000
    return hashlib.pbkdf2_hmac("sha256", password, salt, rounds).hex()


def pbkdf2_verify(payload):
    expected = _text(payload.get("expected", ""))
    actual = pbkdf2_sha256(payload)
    return 1 if hmac.compare_digest(actual, expected) else 0


def parse_int(payload):
    text = str(payload.get("text", "0")).strip()
    fallback = int(payload.get("default", 0))
    try:
        return int(text)
    except ValueError:
        return fallback


def http_request(payload):
    method = payload.get("method", "GET").upper()
    url = payload["url"]
    body = payload.get("body", "")
    headers = payload.get("headers", {})
    timeout = float(payload.get("timeout", 10))
    data = body.encode("utf-8") if body else None
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as response:
            return {
                "status": response.status,
                "body": response.read().decode("utf-8", errors="replace"),
                "headers": dict(response.headers.items()),
            }
    except urllib.error.HTTPError as exc:
        return {
            "status": exc.code,
            "body": exc.read().decode("utf-8", errors="replace"),
            "headers": dict(exc.headers.items()),
        }


class _NoRedirectHandler(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        raise urllib.error.HTTPError(newurl, code, msg, headers, fp)


def safe_http_request(payload):
    method = payload.get("method", "GET").upper()
    url = payload.get("url", "")
    body = payload.get("body", "")
    timeout = float(payload.get("timeout", 10))
    max_bytes = int(payload.get("max_bytes", 262144))
    content_type = payload.get("content_type", "")
    timeout = min(max(timeout, 1.0), 30.0)
    max_bytes = min(max(max_bytes, 1024), 1048576)
    parsed = urllib.parse.urlparse(url)
    if parsed.scheme != "https":
        return {"ok": False, "error": "https is required"}
    if not parsed.netloc:
        return {"ok": False, "error": "missing https host"}
    headers = {"User-Agent": "Spear/0.1"}
    if content_type:
        headers["Content-Type"] = content_type
    data = body.encode("utf-8") if body else None
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    opener = urllib.request.build_opener(_NoRedirectHandler)
    try:
        with opener.open(req, timeout=timeout) as response:
            data = response.read(max_bytes + 1)
            if len(data) > max_bytes:
                return {"ok": False, "error": "response body too large"}
            return {
                "ok": True,
                "status": response.status,
                "body": data.decode("utf-8", errors="replace"),
                "headers": dict(response.headers.items()),
                "url": url,
            }
    except urllib.error.HTTPError as exc:
        body_bytes = exc.read(max_bytes + 1)
        if len(body_bytes) > max_bytes:
            return {"ok": False, "error": "response body too large"}
        return {
            "ok": False,
            "status": exc.code,
            "body": body_bytes.decode("utf-8", errors="replace"),
            "error": f"http status {exc.code}",
            "headers": dict(exc.headers.items()),
            "url": url,
        }
    except Exception as exc:  # noqa: BLE001
        return {"ok": False, "error": str(exc), "url": url}


def http_serve_text(payload):
    host = payload.get("host", "127.0.0.1")
    port = int(payload.get("port", 4173))
    body = payload.get("body", "")
    content_type = payload.get("content_type", "text/plain; charset=utf-8")
    status = int(payload.get("status", 200))

    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            encoded = body.encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(encoded)))
            self.end_headers()
            self.wfile.write(encoded)

        def log_message(self, _format, *_args):
            return

    with ThreadingHTTPServer((host, port), Handler) as server:
        server.serve_forever()


def http_serve_directory(payload):
    import functools
    import http.server

    host = payload.get("host", "127.0.0.1")
    port = int(payload.get("port", 4173))
    directory = payload.get("directory", ".")
    handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=directory)
    with ThreadingHTTPServer((host, port), handler) as server:
        server.serve_forever()


def tcp_request(payload):
    host = payload["host"]
    port = int(payload["port"])
    message = payload.get("message", "").encode("utf-8")
    timeout = float(payload.get("timeout", 3))
    with socket.create_connection((host, port), timeout=timeout) as sock:
        if message:
            sock.sendall(message)
        sock.shutdown(socket.SHUT_WR)
        chunks = []
        while True:
            data = sock.recv(4096)
            if not data:
                break
            chunks.append(data)
    return b"".join(chunks).decode("utf-8", errors="replace")


def udp_send(payload):
    host = payload["host"]
    port = int(payload["port"])
    message = payload.get("message", "").encode("utf-8")
    timeout = float(payload.get("timeout", 3))
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(timeout)
        sock.sendto(message, (host, port))
        if payload.get("expect_reply"):
            data, _ = sock.recvfrom(4096)
            return data.decode("utf-8", errors="replace")
    return ""


def random_int(payload):
    start = int(payload.get("start", 0))
    end = int(payload.get("end", 100))
    return random.randint(start, end)


def random_float(payload):
    start = float(payload.get("start", 0.0))
    end = float(payload.get("end", 1.0))
    return start + (end - start) * random.random()


def stats_summary(payload):
    values = [float(v) for v in payload.get("values", [])]
    if not values:
        return {"count": 0, "sum": 0, "mean": 0, "median": 0, "min": 0, "max": 0}
    return {
        "count": len(values),
        "sum": sum(values),
        "mean": statistics.fmean(values),
        "median": statistics.median(values),
        "min": min(values),
        "max": max(values),
    }


def vector_dot(payload):
    left = [float(v) for v in payload.get("left", [])]
    right = [float(v) for v in payload.get("right", [])]
    return sum(a * b for a, b in zip(left, right))


def vector_norm(payload):
    values = [float(v) for v in payload.get("values", [])]
    return math.sqrt(sum(v * v for v in values))


def vector_add(payload):
    left = [float(v) for v in payload.get("left", [])]
    right = [float(v) for v in payload.get("right", [])]
    return [a + b for a, b in zip(left, right)]


def sha256_hex(payload):
    return hashlib.sha256(payload.get("text", "").encode("utf-8")).hexdigest()


def hmac_sha256(payload):
    key = payload.get("key", "").encode("utf-8")
    text = payload.get("text", "").encode("utf-8")
    return hmac.new(key, text, hashlib.sha256).hexdigest()


def jwt_hs256(payload):
    header = {"alg": "HS256", "typ": "JWT"}
    body = payload.get("claims", {})
    key = payload.get("key", "").encode("utf-8")

    def encode_part(obj):
        raw = json.dumps(obj, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
        return base64.urlsafe_b64encode(raw).rstrip(b"=").decode("ascii")

    first = encode_part(header)
    second = encode_part(body)
    signing = f"{first}.{second}".encode("ascii")
    sig = base64.urlsafe_b64encode(hmac.new(key, signing, hashlib.sha256).digest()).rstrip(b"=").decode("ascii")
    return f"{first}.{second}.{sig}"


def jwt_decode(payload):
    token = payload.get("token", "")
    parts = token.split(".")
    if len(parts) < 2:
        return {}

    def decode_part(text):
        pad = "=" * (-len(text) % 4)
        raw = base64.urlsafe_b64decode((text + pad).encode("ascii"))
        return json.loads(raw.decode("utf-8"))

    return {"header": decode_part(parts[0]), "claims": decode_part(parts[1])}


def jwt_verify_hs256(payload):
    token = payload.get("token", "")
    key = payload.get("key", "").encode("utf-8")
    leeway = int(payload.get("leeway", 0))
    parts = token.split(".")
    if len(parts) != 3:
        return {"ok": False, "error": "invalid token format"}

    def decode_part(text):
        pad = "=" * (-len(text) % 4)
        raw = base64.urlsafe_b64decode((text + pad).encode("ascii"))
        return json.loads(raw.decode("utf-8"))

    try:
        header = decode_part(parts[0])
        claims = decode_part(parts[1])
    except Exception:  # noqa: BLE001
        return {"ok": False, "error": "invalid token payload"}

    if not isinstance(header, dict) or header.get("alg") != "HS256":
        return {"ok": False, "error": "unsupported jwt algorithm"}
    signing = f"{parts[0]}.{parts[1]}".encode("ascii")
    expected = base64.urlsafe_b64encode(hmac.new(key, signing, hashlib.sha256).digest()).rstrip(b"=").decode("ascii")
    if not hmac.compare_digest(expected, parts[2]):
        return {"ok": False, "error": "invalid jwt signature"}
    now = int(time.time())
    if isinstance(claims, dict):
        exp = claims.get("exp")
        nbf = claims.get("nbf")
        if isinstance(exp, (int, float)) and now > int(exp) + leeway:
            return {"ok": False, "error": "jwt has expired"}
        if isinstance(nbf, (int, float)) and now + leeway < int(nbf):
            return {"ok": False, "error": "jwt is not active yet"}
    return {"ok": True, "claims": claims}


def _resolve_under_root(root_text, relative_text):
    root = Path(root_text).resolve()
    target = (root / relative_text).resolve()
    try:
        target.relative_to(root)
    except ValueError as exc:
        raise RuntimeError("path escapes root") from exc
    return target


def fs_safe_read(payload):
    path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    if not path.exists() or not path.is_file():
        return {"ok": False, "error": "file was not found"}
    return {"ok": True, "text": path.read_text(encoding="utf-8"), "path": str(path)}


def fs_safe_write(payload):
    path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(payload.get("content", ""), encoding="utf-8")
    return {"ok": True, "path": str(path)}


def fs_safe_list(payload):
    path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    if not path.exists() or not path.is_dir():
        return {"ok": False, "error": "directory was not found"}
    return {"ok": True, "items": [item.name for item in path.iterdir()], "path": str(path)}


def fs_safe_mkdir(payload):
    path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    path.mkdir(parents=True, exist_ok=True)
    return {"ok": True, "path": str(path)}


def fs_safe_remove(payload):
    path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    if path.is_dir():
        if any(path.iterdir()):
            return {"ok": False, "error": "refusing to remove non-empty directory"}
        path.rmdir()
        return {"ok": True, "path": str(path)}
    if path.exists():
        path.unlink()
        return {"ok": True, "path": str(path)}
    return {"ok": False, "error": "path was not found"}


def sqlite_execute(payload):
    path = payload["path"]
    sql = payload["sql"]
    params = payload.get("params", [])
    with sqlite3.connect(path) as conn:
        cur = conn.execute(sql, params)
        conn.commit()
        return {"rows_affected": cur.rowcount}


def sqlite_query(payload):
    path = payload["path"]
    sql = payload["sql"]
    params = payload.get("params", [])
    with sqlite3.connect(path) as conn:
        conn.row_factory = sqlite3.Row
        cur = conn.execute(sql, params)
        return [dict(row) for row in cur.fetchall()]


def config_load(payload):
    path = Path(payload["path"])
    text = path.read_text(encoding="utf-8")
    suffix = path.suffix.lower()
    if suffix == ".json":
        return json.loads(text)
    if suffix == ".toml" and tomllib is not None:
        return tomllib.loads(text)
    if suffix == ".csv":
        return csv_parse({"text": text})
    return {"text": text}


def logger_line(payload):
    level = payload.get("level", "INFO").upper()
    message = payload.get("message", "")
    stamp = time.strftime("%Y-%m-%d %H:%M:%S")
    return f"[{stamp}] {level} {message}"


def _html_escape(text):
    return (
        _text(text)
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def _compose_string(text):
    value = _text(text)
    value = value.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
    return f"\"{value}\""


def _ui_kind(node):
    return node.get("type", "")


def _ui_items(node):
    items = node.get("items", [])
    return items if isinstance(items, list) else []


def _ui_body(node):
    body = node.get("body", {})
    return body if isinstance(body, dict) else {}


def _ui_text_value(node, key, fallback=""):
    return _text(node.get(key, fallback))


def _ui_state_attrs(states):
    attrs = []
    for item in states:
        name = _html_escape(item.get("name", "state"))
        attrs.append(f'data-state-{name}="{_html_escape(item.get("value", ""))}"')
    return (" " + " ".join(attrs)) if attrs else ""


def _ui_render_web_items(items):
    return "".join(_ui_render_web_node(item) for item in items)


def _ui_render_web_node(node):
    kind = _ui_kind(node)
    if kind == "column":
        body = _ui_render_web_items(_ui_items(node))
        return f'<div style="display:flex;flex-direction:column;gap:16px">{body}</div>'
    if kind == "row":
        body = _ui_render_web_items(_ui_items(node))
        return f'<div style="display:flex;gap:16px;flex-wrap:wrap;align-items:flex-start">{body}</div>'
    if kind == "form":
        body = _ui_render_web_items(_ui_items(node))
        return f'<form style="display:flex;flex-direction:column;gap:16px">{body}</form>'
    if kind == "nav":
        body = _ui_render_web_items(_ui_items(node))
        return f'<nav style="display:flex;gap:12px;flex-wrap:wrap;align-items:center">{body}</nav>'
    if kind == "title":
        return f'<h1 style="font-size:40px;line-height:1.0;letter-spacing:-0.04em">{_html_escape(node.get("text", ""))}</h1>'
    if kind == "text":
        return f'<p style="font-size:18px;line-height:1.7;color:#4b5563">{_html_escape(node.get("text", ""))}</p>'
    if kind == "notice":
        tone = _html_escape(node.get("tone", "neutral"))
        title = _html_escape(node.get("title", "Notice"))
        body = _html_escape(node.get("text", ""))
        border = "#2f6fed" if tone == "info" else "#dc2626" if tone == "error" else "#d97706" if tone == "warning" else "#6b7280"
        bg = "#eff6ff" if tone == "info" else "#fef2f2" if tone == "error" else "#fffbeb" if tone == "warning" else "#f9fafb"
        return f'<aside style="padding:16px;border-left:4px solid {border};background:{bg};display:flex;flex-direction:column;gap:8px"><strong>{title}</strong><p style="margin:0;color:#374151">{body}</p></aside>'
    if kind == "loading":
        body = _html_escape(node.get("text", "Loading"))
        return f'<section style="padding:18px;border:1px dashed #cbd5e1;border-radius:18px;background:#ffffff;color:#6b7280">{body}...</section>'
    if kind == "empty":
        title = _html_escape(node.get("title", "Empty"))
        body = _html_escape(node.get("text", ""))
        return f'<section style="padding:22px;border:1px dashed #d1d5db;border-radius:20px;background:#ffffff"><h3 style="margin:0 0 8px 0">{title}</h3><p style="margin:0;color:#6b7280">{body}</p></section>'
    if kind == "error":
        title = _html_escape(node.get("title", "Error"))
        body = _html_escape(node.get("text", ""))
        return f'<section style="padding:22px;border:1px solid #fecaca;border-radius:20px;background:#fff1f2"><h3 style="margin:0 0 8px 0;color:#991b1b">{title}</h3><p style="margin:0;color:#7f1d1d">{body}</p></section>'
    if kind == "button":
        label = _html_escape(node.get("label", "Action"))
        action = _html_escape(node.get("action", "action"))
        return f'<button data-action="{action}" style="padding:12px 18px;border:0;border-radius:14px;background:#111827;color:#ffffff;font:inherit;cursor:pointer">{label}</button>'
    if kind == "link":
        label = _html_escape(node.get("label", "Open"))
        route = _html_escape(node.get("route", "#"))
        return f'<a href="{route}" style="display:inline-block;padding:12px 18px;border-radius:14px;border:1px solid #d1d5db;color:#111827;text-decoration:none">{label}</a>'
    if kind == "input":
        label = _html_escape(node.get("label", "Field"))
        state = _html_escape(node.get("state", "value"))
        placeholder = _html_escape(node.get("placeholder", ""))
        value = _html_escape(node.get("value", ""))
        return (
            '<label style="display:flex;flex-direction:column;gap:8px">'
            f'<span style="font-size:14px;color:#374151">{label}</span>'
            f'<input name="{state}" value="{value}" placeholder="{placeholder}" '
            'style="padding:12px 14px;border-radius:12px;border:1px solid #d1d5db;background:#ffffff;color:#111827;font:inherit" />'
            "</label>"
        )
    if kind == "stat":
        label = _html_escape(node.get("label", "Metric"))
        value = _html_escape(node.get("value", "0"))
        return (
            '<section style="padding:18px;border:1px solid #e5e7eb;border-radius:18px;background:#ffffff;min-width:180px">'
            f'<p style="margin:0 0 8px 0;font-size:14px;color:#6b7280">{label}</p>'
            f'<h3 style="margin:0;font-size:28px;line-height:1.05">{value}</h3>'
            "</section>"
        )
    if kind == "card":
        title = _html_escape(node.get("title", "Card"))
        body = _ui_render_web_node(_ui_body(node))
        return (
            '<section style="padding:24px;border:1px solid #e5e7eb;border-radius:22px;background:#ffffff">'
            f'<h2 style="margin:0 0 16px 0;font-size:24px;line-height:1.1">{title}</h2>{body}</section>'
        )
    if kind == "section":
        title = _html_escape(node.get("title", "Section"))
        body = _ui_render_web_node(_ui_body(node))
        return (
            '<section style="display:flex;flex-direction:column;gap:16px">'
            f'<h2 style="margin:0;font-size:28px;line-height:1.1">{title}</h2>{body}</section>'
        )
    return ""


def ui_web_render(payload):
    spec = payload if isinstance(payload, dict) else json.loads(payload or "{}")
    if isinstance(spec.get("body"), dict) and spec.get("body", {}).get("type") == "screen":
        spec = spec["body"]
    title = _html_escape(spec.get("title", "Spear UI"))
    attrs = _ui_state_attrs(spec.get("states", []))
    body = _ui_render_web_node(_ui_body(spec))
    return (
        "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        f"<title>{title}</title>"
        "<style>*{box-sizing:border-box}html,body{margin:0;padding:0}body{font-family:Inter,Segoe UI,sans-serif;background:#f5f3ee;color:#111827}"
        "main{max-width:1080px;margin:0 auto;padding:28px 22px 72px 22px}button,input{font:inherit}</style></head>"
        f"<body><main{attrs}>{body}</main></body></html>"
    )


def _ui_render_android_node(node, indent):
    pad = " " * indent
    kind = _ui_kind(node)
    if kind == "column":
        head = (
            f"{pad}Column(\n"
            f"{pad}    verticalArrangement = Arrangement.spacedBy(16.dp)\n"
            f"{pad}) {{\n"
        )
        body = "".join(_ui_render_android_node(item, indent + 4) for item in _ui_items(node))
        return f"{head}{body}{pad}}}\n"
    if kind == "row":
        head = (
            f"{pad}Row(\n"
            f"{pad}    horizontalArrangement = Arrangement.spacedBy(16.dp)\n"
            f"{pad}) {{\n"
        )
        body = "".join(_ui_render_android_node(item, indent + 4) for item in _ui_items(node))
        return f"{head}{body}{pad}}}\n"
    if kind == "form":
        head = (
            f"{pad}Column(\n"
            f"{pad}    verticalArrangement = Arrangement.spacedBy(16.dp)\n"
            f"{pad}) {{\n"
        )
        body = "".join(_ui_render_android_node(item, indent + 4) for item in _ui_items(node))
        return f"{head}{body}{pad}}}\n"
    if kind == "nav":
        head = (
            f"{pad}Row(\n"
            f"{pad}    horizontalArrangement = Arrangement.spacedBy(12.dp)\n"
            f"{pad}) {{\n"
        )
        body = "".join(_ui_render_android_node(item, indent + 4) for item in _ui_items(node))
        return f"{head}{body}{pad}}}\n"
    if kind == "title":
        return f"{pad}Text(text = {_compose_string(node.get('text', ''))}, style = MaterialTheme.typography.headlineLarge)\n"
    if kind == "text":
        return f"{pad}Text(text = {_compose_string(node.get('text', ''))}, style = MaterialTheme.typography.bodyLarge)\n"
    if kind == "notice":
        title = _compose_string(node.get("title", "Notice"))
        text = _compose_string(node.get("text", ""))
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {title}, style = MaterialTheme.typography.titleMedium)\n"
            f"{pad}        Text(text = {text}, style = MaterialTheme.typography.bodyMedium)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "loading":
        text = _compose_string(node.get("text", "Loading"))
        return f"{pad}Text(text = {text}, style = MaterialTheme.typography.bodyMedium)\n"
    if kind == "empty":
        title = _compose_string(node.get("title", "Empty"))
        text = _compose_string(node.get("text", ""))
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {title}, style = MaterialTheme.typography.titleMedium)\n"
            f"{pad}        Text(text = {text}, style = MaterialTheme.typography.bodyMedium)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "error":
        title = _compose_string(node.get("title", "Error"))
        text = _compose_string(node.get("text", ""))
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {title}, style = MaterialTheme.typography.titleMedium)\n"
            f"{pad}        Text(text = {text}, style = MaterialTheme.typography.bodyMedium)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "button":
        label = _compose_string(node.get("label", "Action"))
        action = _text(node.get("action", "action"))
        return f"{pad}Button(onClick = {{ /* {action} */ }}) {{ Text(text = {label}) }}\n"
    if kind == "link":
        label = _compose_string(node.get("label", "Open"))
        route = _text(node.get("route", "route"))
        return f"{pad}TextButton(onClick = {{ /* go {route} */ }}) {{ Text(text = {label}) }}\n"
    if kind == "input":
        state = _text(node.get("state", "value"))
        label = _compose_string(node.get("label", "Field"))
        placeholder = _compose_string(node.get("placeholder", ""))
        return (
            f"{pad}OutlinedTextField(\n"
            f"{pad}    value = {state},\n"
            f"{pad}    onValueChange = {{ {state} = it }},\n"
            f"{pad}    label = {{ Text(text = {label}) }},\n"
            f"{pad}    placeholder = {{ Text(text = {placeholder}) }},\n"
            f"{pad}    modifier = Modifier.fillMaxWidth()\n"
            f"{pad})\n"
        )
    if kind == "stat":
        label = _compose_string(node.get("label", "Metric"))
        value = _compose_string(node.get("value", "0"))
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {label}, style = MaterialTheme.typography.labelMedium)\n"
            f"{pad}        Text(text = {value}, style = MaterialTheme.typography.headlineSmall)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "card":
        title = _compose_string(node.get("title", "Card"))
        body = _ui_render_android_node(_ui_body(node), indent + 8)
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(14.dp)) {{\n"
            f"{pad}        Text(text = {title}, style = MaterialTheme.typography.titleLarge)\n"
            f"{body}"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "section":
        title = _compose_string(node.get("title", "Section"))
        body = _ui_render_android_node(_ui_body(node), indent + 4)
        return (
            f"{pad}Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {{\n"
            f"{pad}    Text(text = {title}, style = MaterialTheme.typography.titleLarge)\n"
            f"{body}"
            f"{pad}}}\n"
        )
    return ""


def ui_android_render(payload):
    spec = payload if isinstance(payload, dict) else json.loads(payload or "{}")
    screen_name = _text(spec.get("screen_name", "MainScreen")) or "MainScreen"
    if isinstance(spec.get("body"), dict) and spec.get("body", {}).get("type") == "screen":
        screen = spec["body"]
        screen_name = _text(spec.get("screen_name", screen_name)) or screen_name
        spec = screen
    state_lines = []
    for item in spec.get("states", []):
        name = _text(item.get("name", "state")) or "state"
        value = item.get("value", "")
        state_type = item.get("state_type", "text")
        if state_type == "num":
            state_lines.append(f"    var {name} by remember {{ mutableIntStateOf({int(value)}) }}\n")
        else:
            state_lines.append(f"    var {name} by remember {{ mutableStateOf({_compose_string(value)}) }}\n")
    states = "".join(state_lines)
    body = _ui_render_android_node(_ui_body(spec), 8)
    return (
        "import androidx.compose.foundation.layout.*\n"
        "import androidx.compose.material3.*\n"
        "import androidx.compose.runtime.*\n"
        "import androidx.compose.ui.Modifier\n"
        "import androidx.compose.ui.graphics.Color\n"
        "import androidx.compose.ui.unit.dp\n\n"
        f"@Composable\nfun {screen_name}() {{\n"
        f"{states}"
        "    Surface(modifier = Modifier.fillMaxSize(), color = Color(0xFFF5F3EE)) {\n"
        "        Column(\n"
        "            modifier = Modifier.fillMaxSize().padding(24.dp),\n"
        "            verticalArrangement = Arrangement.spacedBy(16.dp)\n"
        "        ) {\n"
        f"{body}"
        "        }\n"
        "    }\n"
        "}\n"
    )
