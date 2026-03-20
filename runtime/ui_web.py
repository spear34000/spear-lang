from __future__ import annotations

from ui_common import (
    action_attr_map,
    html_attr_string,
    html_escape,
    main_attrs,
    node_body,
    node_items,
    node_kind,
    screen_meta,
    state_lookup,
    text_value,
)


STYLES = {
    "body": "font-family:Segoe UI,Arial,sans-serif;background:#f5f3ee;color:#111827",
    "main": "max-width:1080px;margin:0 auto;padding:28px 22px 72px 22px;display:flex;flex-direction:column;gap:20px",
    "column": "display:flex;flex-direction:column;gap:16px",
    "row": "display:flex;gap:16px;flex-wrap:wrap;align-items:flex-start",
    "nav": "display:flex;gap:12px;flex-wrap:wrap;align-items:center",
    "toolbar": "display:flex;justify-content:space-between;gap:16px;align-items:center;padding-bottom:8px;border-bottom:1px solid #e5e7eb",
    "tabs": "display:flex;gap:10px;flex-wrap:wrap;align-items:center",
    "tab": "padding:10px 14px;border:1px solid #d1d5db;border-radius:999px;background:#ffffff;color:#111827;text-decoration:none",
    "list": "display:flex;flex-direction:column;gap:12px;list-style:none;margin:0;padding:0",
    "item": "padding:16px 18px;border:1px solid #e5e7eb;border-radius:18px;background:#ffffff;display:flex;flex-direction:column;gap:8px",
    "form": "display:flex;flex-direction:column;gap:16px",
    "title": "font-size:40px;line-height:1;letter-spacing:-0.04em;margin:0",
    "text": "font-size:18px;line-height:1.7;color:#4b5563;margin:0",
    "divider": "border:0;border-top:1px solid #e5e7eb;margin:0",
    "button": "padding:12px 18px;border:0;border-radius:14px;background:#111827;color:#ffffff;font:inherit;cursor:pointer",
    "button_secondary": "display:inline-block;padding:12px 18px;border-radius:14px;border:1px solid #d1d5db;color:#111827;text-decoration:none;background:#ffffff",
    "field": "padding:12px 14px;border-radius:12px;border:1px solid #d1d5db;background:#ffffff;color:#111827;font:inherit;width:100%",
    "field_error": "padding:12px 14px;border-radius:12px;border:1px solid #dc2626;background:#ffffff;color:#111827;font:inherit;width:100%",
    "card": "padding:24px;border:1px solid #e5e7eb;border-radius:22px;background:#ffffff;display:flex;flex-direction:column;gap:16px",
    "section": "display:flex;flex-direction:column;gap:16px",
    "dialog": "padding:22px;border:1px solid #cbd5e1;border-radius:20px;background:#ffffff;box-shadow:0 24px 60px rgba(15,23,42,0.14);display:flex;flex-direction:column;gap:16px",
    "sheet": "padding:22px;border:1px solid #e5e7eb;border-radius:24px 24px 0 0;background:#ffffff;display:flex;flex-direction:column;gap:16px",
    "stat": "padding:18px;border:1px solid #e5e7eb;border-radius:18px;background:#ffffff;min-width:180px;display:flex;flex-direction:column;gap:8px",
}


def _render_children(items, states):
    return "".join(_render_node(item, states) for item in items)


def _render_input(node, states):
    label = html_escape(node.get("label", "Field"))
    state_name = text_value(node.get("state", "value"))
    state = states.get(state_name, {})
    value = html_escape(state.get("value", node.get("value", "")))
    placeholder = html_escape(node.get("placeholder", ""))
    hint = text_value(node.get("hint", ""))
    error_text = text_value(node.get("error_text", ""))
    required = bool(node.get("required", False))
    disabled = bool(node.get("disabled", False))
    kind = text_value(node.get("kind", "text")) or "text"
    attrs = {
        "name": html_escape(state_name),
        "placeholder": placeholder,
        "value": value,
        "data-state-target": html_escape(state_name),
        "data-kind": html_escape(kind),
    }
    if required:
        attrs["required"] = "required"
    if disabled:
        attrs["disabled"] = "disabled"
        attrs["aria-disabled"] = "true"
    tag = "textarea" if kind == "multiline" else "input"
    field_style = STYLES["field_error"] if error_text else STYLES["field"]
    if tag == "textarea":
        field = (
            f'<textarea style="{field_style};min-height:120px"'
            f'{html_attr_string(attrs)}>{value}</textarea>'
        )
    else:
        attrs["type"] = html_escape(kind if kind in ("text", "email", "password", "search") else "text")
        field = f'<input style="{field_style}"{html_attr_string(attrs)} />'
    parts = [
        '<label style="display:flex;flex-direction:column;gap:8px">',
        f'<span style="font-size:14px;color:#374151">{label}</span>',
        field,
    ]
    if hint:
        parts.append(f'<small style="color:#6b7280">{html_escape(hint)}</small>')
    if error_text:
        parts.append(f'<small role="alert" style="color:#991b1b">{html_escape(error_text)}</small>')
    parts.append("</label>")
    return "".join(parts)


def _render_node(node, states):
    kind = node_kind(node)
    if kind == "column":
        return f'<div style="{STYLES["column"]}">{_render_children(node_items(node), states)}</div>'
    if kind == "row":
        return f'<div style="{STYLES["row"]}">{_render_children(node_items(node), states)}</div>'
    if kind == "form":
        attrs = {"aria-label": html_escape(node.get("title", "Form"))}
        action = node.get("action")
        if action:
            attrs.update(action_attr_map(action))
        return f'<form style="{STYLES["form"]}"{html_attr_string(attrs)}>{_render_children(node_items(node), states)}</form>'
    if kind == "nav":
        return f'<nav aria-label="Primary" style="{STYLES["nav"]}">{_render_children(node_items(node), states)}</nav>'
    if kind == "toolbar":
        left = _render_children(node_items({"items": node.get("left", [])}), states)
        right = _render_children(node_items({"items": node.get("right", [])}), states)
        title = text_value(node.get("title", ""))
        middle = f'<strong style="font-size:15px">{html_escape(title)}</strong>' if title else ""
        return f'<header style="{STYLES["toolbar"]}"><div style="{STYLES["nav"]}">{left}</div>{middle}<div style="{STYLES["nav"]}">{right}</div></header>'
    if kind == "tabs":
        return f'<nav aria-label="Tabs" role="tablist" style="{STYLES["tabs"]}">{_render_children(node_items(node), states)}</nav>'
    if kind == "list":
        return f'<ul role="list" style="{STYLES["list"]}">{_render_children(node_items(node), states)}</ul>'
    if kind == "item":
        title = text_value(node.get("title", ""))
        body = _render_node(node_body(node), states)
        title_html = f'<strong>{html_escape(title)}</strong>' if title else ""
        return f'<li style="{STYLES["item"]}">{title_html}{body}</li>'
    if kind == "title":
        return f'<h1 style="{STYLES["title"]}">{html_escape(node.get("text", ""))}</h1>'
    if kind == "text":
        return f'<p style="{STYLES["text"]}">{html_escape(node.get("text", ""))}</p>'
    if kind == "notice":
        tone = html_escape(node.get("tone", "neutral"))
        title = html_escape(node.get("title", "Notice"))
        body = html_escape(node.get("text", ""))
        border = "#2f6fed" if tone == "info" else "#dc2626" if tone == "error" else "#d97706" if tone == "warning" else "#6b7280"
        bg = "#eff6ff" if tone == "info" else "#fef2f2" if tone == "error" else "#fffbeb" if tone == "warning" else "#f9fafb"
        return f'<aside style="padding:16px;border-left:4px solid {border};background:{bg};display:flex;flex-direction:column;gap:8px"><strong>{title}</strong><p style="margin:0;color:#374151">{body}</p></aside>'
    if kind == "loading":
        return f'<section aria-live="polite" style="padding:18px;border:1px dashed #cbd5e1;border-radius:18px;background:#ffffff;color:#6b7280">{html_escape(node.get("text", "Loading"))}...</section>'
    if kind == "empty":
        return f'<section style="padding:22px;border:1px dashed #d1d5db;border-radius:20px;background:#ffffff"><h3 style="margin:0 0 8px 0">{html_escape(node.get("title", "Empty"))}</h3><p style="margin:0;color:#6b7280">{html_escape(node.get("text", ""))}</p></section>'
    if kind == "error":
        return f'<section role="alert" style="padding:22px;border:1px solid #fecaca;border-radius:20px;background:#fff1f2"><h3 style="margin:0 0 8px 0;color:#991b1b">{html_escape(node.get("title", "Error"))}</h3><p style="margin:0;color:#7f1d1d">{html_escape(node.get("text", ""))}</p></section>'
    if kind == "button":
        attrs = action_attr_map(node.get("action", {}))
        if bool(node.get("disabled", False)):
            attrs["disabled"] = "disabled"
            attrs["aria-disabled"] = "true"
        return f'<button type="button" style="{STYLES["button"]}"{html_attr_string(attrs)}>{html_escape(node.get("label", "Action"))}</button>'
    if kind == "link":
        attrs = action_attr_map(node.get("action", {"type": "go", "route": node.get("route", "#")}))
        route = html_escape(node.get("route", "#"))
        role = "tab" if bool(node.get("tab", False)) else ""
        if role:
            attrs["role"] = role
        style = STYLES["tab"] if role == "tab" else STYLES["button_secondary"]
        return f'<a href="{route}" style="{style}"{html_attr_string(attrs)}>{html_escape(node.get("label", "Open"))}</a>'
    if kind == "input":
        return _render_input(node, states)
    if kind == "stat":
        return f'<section style="{STYLES["stat"]}"><p style="margin:0;font-size:14px;color:#6b7280">{html_escape(node.get("label", "Metric"))}</p><h3 style="margin:0;font-size:28px;line-height:1.05">{html_escape(node.get("value", "0"))}</h3></section>'
    if kind == "card":
        title = text_value(node.get("title", ""))
        title_html = f'<h2 style="margin:0;font-size:24px;line-height:1.1">{html_escape(title)}</h2>' if title else ""
        return f'<section style="{STYLES["card"]}">{title_html}{_render_node(node_body(node), states)}</section>'
    if kind == "section":
        title = text_value(node.get("title", ""))
        title_html = f'<h2 style="margin:0;font-size:28px;line-height:1.1">{html_escape(title)}</h2>' if title else ""
        return f'<section style="{STYLES["section"]}">{title_html}{_render_node(node_body(node), states)}</section>'
    if kind == "dialog":
        title = text_value(node.get("title", "Dialog"))
        attrs = {"role": "dialog", "aria-modal": "true"}
        attrs.update(action_attr_map(node.get("action", {"type": "open_dialog", "dialog": title})))
        return f'<section style="{STYLES["dialog"]}"{html_attr_string(attrs)}><h2 style="margin:0">{html_escape(title)}</h2>{_render_node(node_body(node), states)}</section>'
    if kind == "sheet":
        title = text_value(node.get("title", "Sheet"))
        attrs = {"role": "region", "aria-label": html_escape(title)}
        return f'<section style="{STYLES["sheet"]}"{html_attr_string(attrs)}><h2 style="margin:0">{html_escape(title)}</h2>{_render_node(node_body(node), states)}</section>'
    if kind == "divider":
        return f'<hr style="{STYLES["divider"]}" />'
    return ""


def ui_web_render(payload):
    meta = screen_meta(payload, "HomeScreen")
    spec = meta["screen"]
    title = html_escape(meta["title"] or "Spear UI")
    attrs = main_attrs(spec)
    body = _render_node(node_body(spec), state_lookup(spec))
    return (
        "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        f"<title>{title}</title>"
        "<style>*{box-sizing:border-box}html,body{margin:0;padding:0}h1,h2,h3,p,ul,li,blockquote,pre{margin:0;padding:0}ul{padding-left:0}img,svg,video,canvas{display:block;max-width:100%}table{border-collapse:collapse;border-spacing:0}button,input,textarea,select{font:inherit;color:inherit}body{"
        f"{STYLES['body']}"
        "}main{"
        f"{STYLES['main']}"
        "}</style></head>"
        f"<body><main{html_attr_string(attrs)}>{body}</main></body></html>"
    )
