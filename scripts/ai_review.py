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
Provide concise, professional, and actionable feedback in English.

Strictly enforce layered architecture principles and maintain an evidence-driven mindset.

Rules:
1. Base your analysis solely on the provided git diff. Do not invent facts or extrapolate beyond the scope of changes.
2. Exception to Rule 1: You are explicitly required to identify "missing" security or safety elements directly related to the changed code (e.g., missing error handling, missing null-pointer/status checks, missing test coverage, or unsafe memory operations).
3. Do not claim to have read, executed, or tested the entire repository.
4. Eliminate polite pleasantries, apologies, and full implementation rewrites. Focus strictly on architectural risks and findings.
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