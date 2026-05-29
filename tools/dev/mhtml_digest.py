#!/usr/bin/env python3
"""Digest a saved-as-MHTML web page into a compact structural outline.

Used to understand the TU-Sofia E-Student reference screens in
examples/УИСС/*.mhtml so they can be rebuilt as native MPAPP views.

Prints: <title>, heading outline, nav/menu link text, table rows,
form fields, and standalone button/label text — enough to design the
view without wading through base64 image blobs.
"""
import email
import quopri
import re
import sys
from html.parser import HTMLParser


def load_html(path: str) -> str:
    with open(path, "rb") as f:
        msg = email.message_from_binary_file(f)
    for part in msg.walk():
        if part.get_content_type() == "text/html":
            payload = part.get_payload(decode=False)
            enc = part.get("Content-Transfer-Encoding", "").lower()
            raw = payload.encode("utf-8", "replace")
            if enc == "quoted-printable":
                raw = quopri.decodestring(raw)
            return raw.decode("utf-8", "replace")
    return ""


class Outline(HTMLParser):
    BLOCK = {"h1", "h2", "h3", "h4", "h5", "h6", "tr", "li", "td", "th",
             "button", "label", "a", "option", "p"}
    SKIP = {"script", "style", "noscript", "svg"}

    def __init__(self):
        super().__init__()
        self.out = []
        self.stack = []
        self.cur = []
        self.skip_depth = 0
        self.last_tag = None

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        if tag in self.SKIP:
            self.skip_depth += 1
            return
        if tag == "img":
            alt = a.get("alt") or a.get("src", "")[:40]
            if alt:
                self.out.append(f"  [img: {alt}]")
        if tag == "input":
            t = a.get("type", "text")
            name = a.get("name", a.get("id", "?"))
            ph = a.get("placeholder", "")
            val = a.get("value", "")
            self.out.append(f"  <input {t} name={name!r} ph={ph!r} val={val!r}>")
        if tag in self.BLOCK:
            self.flush()
            self.last_tag = tag

    def handle_endtag(self, tag):
        if tag in self.SKIP and self.skip_depth:
            self.skip_depth -= 1
            return
        if tag in self.BLOCK:
            self.flush()

    def handle_data(self, data):
        if self.skip_depth:
            return
        t = data.strip()
        if t:
            self.cur.append(t)

    def flush(self):
        if not self.cur:
            return
        text = " ".join(self.cur).strip()
        text = re.sub(r"\s+", " ", text)
        self.cur = []
        if not text:
            return
        tag = self.last_tag or ""
        prefix = {"h1": "# ", "h2": "## ", "h3": "### ", "h4": "#### ",
                  "h5": "##### ", "h6": "###### ", "li": "- ", "a": "link: ",
                  "button": "[btn] ", "label": "lbl: ", "th": "TH| ",
                  "td": "td| ", "option": "opt: "}.get(tag, "")
        self.out.append(prefix + text)


def digest_one(path: str) -> str:
    html = load_html(path)
    p = Outline()
    p.feed(html)
    p.flush()
    lines = ["=" * 70, f"FILE: {path}", "=" * 70]
    prev = None
    for line in p.out:
        if line == prev:
            continue
        prev = line
        lines.append(line)
    lines.append("")
    return "\n".join(lines)


def main():
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
    out_path = None
    args = list(sys.argv[1:])
    if len(args) >= 2 and args[0] == "-o":
        out_path = args[1]
        args = args[2:]
    chunks = [digest_one(p) for p in args]
    blob = "\n".join(chunks)
    if out_path:
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(blob)
    else:
        print(blob)


if __name__ == "__main__":
    main()
