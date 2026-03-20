from __future__ import annotations

import base64
import csv
import hashlib
import hmac
import json
import math
import os
import random
import socket
import sqlite3
import statistics
import subprocess
import time
import urllib.error
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

from spear_security import (
    fs_safe_list,
    fs_safe_mkdir,
    fs_safe_read,
    fs_safe_remove,
    fs_safe_write,
    jwt_verify_hs256,
    pbkdf2_sha256,
    pbkdf2_verify,
    safe_http_request,
    secure_compare,
    secure_token,
)
from ui_compose import ui_android_render, ui_desktop_render
from ui_web import ui_web_render

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
