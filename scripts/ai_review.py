import sys
import os
from google import genai

api_key = os.environ.get("GEMINI_API_KEY")

if not api_key:
    print("GEMINI_API_KEY is not set")
    sys.exit(1)

if len(sys.argv) != 2:
    print("usage: ai_review.py <diff-file>")
    sys.exit(1)

with open(sys.argv[1], "r", encoding="utf-8") as f:
    diff = f.read()

MAX_DIFF_SIZE = 20000
diff = diff[:MAX_DIFF_SIZE]

SYSTEM_PROMPT = """
You are a senior firmware and UEFI architecture reviewer.
Answer concisely in Traditional Chinese, but KEEP technical terms and firmware nomenclature in English (e.g., Protocol, PPI, SMI, Hob, Register, Callback, Driver, Core, Adapter).

Strictly enforce layered architecture principles. You must maintain an evidence-driven mindset. 

Rules:
1. Only use evidence from the provided git diff. Do not invent facts or extrapolate beyond the scope of changes.
2. Exception to Rule 1: You ARE expected to identify and report "missing" elements directly related to the changed code (e.g., missing error handling, missing null checks, missing test coverage, or unsafe memory operations).
3. Do not claim you have read, executed, or tested the whole repository. 
4. Do not add unrelated commentary, polite pleasantries, apologies, or full implementation rewrites.
"""

prompt = f"""
{SYSTEM_PROMPT}

Review this git diff.

Focus on:
- layered architecture violations
- Core depending on UI or Adapter
- direct UEFI usage inside Core
- unsafe memory operations
- missing test coverage
- risky refactors
- duplicated logic
- missing error handling

Output format:

# AI Review Summary

## Risk
LOW/MEDIUM/HIGH

## Findings
- item
- item

## Suggested Actions
- item
- item

Keep the review concise and actionable.

Git diff:
{diff}
"""

client = genai.Client(api_key=api_key)

response = client.models.generate_content(
    model="gemini-2.5-flash",
    contents=prompt,
)

print(response.text)