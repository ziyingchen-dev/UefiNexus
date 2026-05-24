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

client = genai.Client(api_key=api_key)

response = client.models.generate_content(
    model="gemini-2.5-flash",
    contents=prompt,
)

print(response.text)