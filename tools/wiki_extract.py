#!/usr/bin/python3

import argparse
import bz2
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

SPECIAL_PREFIXES = (
    "Template:",
    "Category:",
    "File:",
    "Wikipedia:",
    "Portal:",
    "Help:",
)


def clean_text(text: str) -> str:
    # remove templates
    text = re.sub(r"\{\{.*?\}\}", "", text, flags=re.DOTALL)
    # [[link|text]] -> text
    text = re.sub(r"\[\[(?:[^|\]]*\|)?([^\]]+)\]\]", r"\1", text)
    # external links
    text = re.sub(r"http\S+", "", text)
    # headings
    text = re.sub(r"==+.*?==+", "", text)
    # collapse whitespace
    text = re.sub(r"\s+", " ", text)
    return text.strip()


def should_skip(title: str, text: str) -> bool:
    if not title or not text:
        return True
    if text.startswith("#REDIRECT"):
        return True
    for prefix in SPECIAL_PREFIXES:
        if title.startswith(prefix):
            return True
    return False


def extract_articles(input_path: str, output_dir: str, max_pages: int | None):
    out_dir = Path(output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    skipped = 0
    context = ET.iterparse(
        bz2.open(input_path, "rb"),
        events=("end",)
    )

    for _, elem in context:
        if not elem.tag.endswith("page"):
            continue
        title = elem.findtext(".//{*}title")
        text = elem.findtext(".//{*}text")
        if should_skip(title, text):
            skipped += 1
            elem.clear()
            continue
        clean = clean_text(text)
        if not clean:
            skipped += 1
            elem.clear()
            continue

        out_file = out_dir / f"{count}.txt"
        with open(out_file, "w", encoding="utf-8") as f:
            f.write(title)
            f.write("\n\n")
            f.write(clean)
        count += 1

        if count % 1000 == 0:
            print(f"[extract] {count} pages extracted")
        if max_pages and count >= max_pages:
            break

        elem.clear()

    print(f"[extract] done extracted={count} skipped={skipped}")

script_name = sys.argv[0]

def main():
    parser = argparse.ArgumentParser(
        description="Extract Wikipedia articles from an XML dump into text files.",
        epilog=(
            "Example:\n"
            f"  python3 {script_name} "
            "--input simplewiki-latest-pages-articles.xml.bz2 "
            "--output data/wiki_text "
            "--max-pages 10000"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--input",
        required=True,
        help="Path to wikipedia XML dump (.bz2)"
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Directory to write extracted text files"
    )
    parser.add_argument(
        "--max-pages",
        type=int,
        default=None,
        help="Maximum pages to extract"
    )
    args = parser.parse_args()
    extract_articles(
        input_path=args.input,
        output_dir=args.output,
        max_pages=args.max_pages
    )


if __name__ == "__main__":
    main()
