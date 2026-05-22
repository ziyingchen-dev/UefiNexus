import sys
import os
import google.generativeai as genai

api_key = os.environ.get("GEMINI_API_KEY")

if not api_key:
    print("GEMINI_API_KEY is not set")
    sys.exit(1)

genai.configure(api_key=api_key)

model = genai.GenerativeModel("gemini-1.5-flash")

if len(sys.argv) != 2:
    print("usage: ai_review.py <diff-file>")
    sys.exit(1)

with open(sys.argv[1], "r", encoding="utf-8") as f:
    diff = f.read()

prompt = f"""
You are a senior firmware and UEFI architecture reviewer.

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

response = model.generate_content(prompt)

print(response.text)