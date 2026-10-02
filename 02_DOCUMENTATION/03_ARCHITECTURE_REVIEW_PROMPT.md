# CLAUDE — ARCHITECTURE REVIEW PROMPT

Inspect the entire FOUNDATION project and all reference files.

Do not implement the product yet.

First:
1. Map the specification into a concrete technical architecture.
2. Propose the class/module structure.
3. Define the DSP graph and signal-flow boundaries.
4. Define the parameter ID strategy.
5. Define state/preset serialization.
6. Define modulation and macro architecture.
7. Define PLAY vs EDIT UI architecture.
8. Identify dependencies between development phases.
9. Identify technical risks and areas requiring validation.
10. Keep the project buildable/testable at every phase.

Return the proposed architecture for review before bulk implementation.

Critical rule:
PLAY is already designed. PLAY is the default landing page.
Do not redesign PLAY. EDIT is where the advanced sound-design system lives.
