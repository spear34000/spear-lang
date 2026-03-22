import importlib
import json
import os
import sys
from pathlib import Path


def main() -> int:
    _, target, fn, req_path, res_path = sys.argv
    try:
        extra = os.environ.get("SHARP_PYTHONPATH", "")
        for entry in reversed([v for v in extra.split(os.pathsep) if v]):
            if entry not in sys.path:
                sys.path.insert(0, entry)
        payload = Path(req_path).read_text(encoding="utf-8")
        module = importlib.import_module(target)
        candidate = getattr(module, fn, None)
        if not callable(candidate):
            raise RuntimeError(f"python bridge: function '{fn}' was not found in '{target}'")
        if payload:
            try:
                arg = json.loads(payload)
            except json.JSONDecodeError:
                arg = payload
        else:
            arg = None
        result = candidate(arg)
        if isinstance(result, str):
            text = result
        else:
            text = json.dumps(result)
        Path(res_path).write_text(text, encoding="utf-8")
        return 0
    except Exception as exc:  # noqa: BLE001
        Path(res_path).write_text(str(exc), encoding="utf-8")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
