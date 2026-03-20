from __future__ import annotations

import json


def text_value(value):
    if value is None:
        return ""
    if isinstance(value, str):
        return value
    return json.dumps(value, ensure_ascii=False)


def html_escape(text):
    return (
        text_value(text)
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def compose_string(text):
    value = text_value(text)
    value = value.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
    return f"\"{value}\""


def parse_payload(payload):
    return payload if isinstance(payload, dict) else json.loads(payload or "{}")


def state_items(spec):
    states = spec.get("states", [])
    return states if isinstance(states, list) else []


def state_lookup(spec):
    lookup = {}
    for item in state_items(spec):
        name = text_value(item.get("name", ""))
        if name:
            lookup[name] = item
    return lookup


def node_kind(node):
    return node.get("type", "")


def node_items(node):
    items = node.get("items", [])
    return items if isinstance(items, list) else []


def node_body(node):
    body = node.get("body", {})
    return body if isinstance(body, dict) else {}


def normalize_action(action):
    if isinstance(action, dict):
        result = dict(action)
    elif isinstance(action, str):
        result = {"type": "tap", "name": action}
    else:
        result = {"type": "tap", "name": ""}
    action_type = text_value(result.get("type", "tap")) or "tap"
    result["type"] = action_type
    if action_type == "go" and "route" not in result:
        result["route"] = text_value(result.get("name", ""))
    return result


def action_attr_map(action):
    action = normalize_action(action)
    attrs = {"data-action": html_escape(action.get("type", "tap"))}
    mapping = {
        "name": "data-action-name",
        "route": "data-route",
        "dialog": "data-dialog",
        "state_target": "data-state-target",
        "state_value": "data-state-value",
    }
    for key, attr_name in mapping.items():
        value = text_value(action.get(key, ""))
        if value:
            attrs[attr_name] = html_escape(value)
    return attrs


def html_attr_string(attrs):
    if not attrs:
        return ""
    ordered = [f'{name}="{value}"' for name, value in attrs.items() if value != ""]
    return (" " + " ".join(ordered)) if ordered else ""


def screen_meta(spec, default_screen_name="MainScreen"):
    spec = parse_payload(spec)
    screen = spec
    screen_name = text_value(spec.get("screen_name", default_screen_name)) or default_screen_name
    if isinstance(spec.get("body"), dict) and spec.get("body", {}).get("type") == "screen":
        screen_name = text_value(spec.get("screen_name", screen_name)) or screen_name
        screen = spec["body"]
    route = text_value(screen.get("route", ""))
    title = text_value(screen.get("title", screen_name))
    return {"screen": screen, "screen_name": screen_name, "route": route, "title": title}


def main_attrs(spec):
    attrs = {}
    for item in state_items(spec):
        name = html_escape(item.get("name", "state"))
        attrs[f"data-state-{name}"] = html_escape(item.get("value", ""))
    route = text_value(spec.get("route", ""))
    title = text_value(spec.get("title", ""))
    if route:
        attrs["data-route"] = html_escape(route)
    if title:
        attrs["data-screen-title"] = html_escape(title)
    return attrs


def compose_action_expression(action):
    action = normalize_action(action)
    parts = [f'"type" to {compose_string(action.get("type", ""))}']
    for key in ("name", "route", "dialog", "state_target", "state_value"):
        value = text_value(action.get(key, ""))
        if value:
            parts.append(f'"{key}" to {compose_string(value)}')
    return "spearAction(" + ", ".join(parts) + ")"
