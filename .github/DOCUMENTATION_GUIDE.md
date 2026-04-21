# Cellular Manager Documentation Guide

## Documentation Structure

```
docs/
├── README.md                  # Navigation hub
├── architecture.md            # System design (HLD + LLD merged)
├── workflows.md               # Runtime operational flows
├── troubleshooting.md         # Diagnosis, decision trees, RCA
├── developer-playbook.md      # Commands & validation
├── onboarding.md              # New engineer guide
└── reference/
    ├── hal-api.md             # HAL API reference
    ├── callbacks.md           # Callback lifecycle
    └── tr181-matrix.md        # TR-181 ownership

.github/
├── knowledge/
│   └── reference-data.md     # Enums, error codes, failure patterns (AI-focused)
├── skills/                    # AI skill definitions
└── instructions/              # Language-specific coding guidelines
```

## Conventions

### Where to Put Documentation

| Type | Location | Audience |
|------|----------|----------|
| System design, architecture | `docs/architecture.md` | Engineers, reviewers |
| Operational flows | `docs/workflows.md` | Engineers |
| Troubleshooting, RCA | `docs/troubleshooting.md` | On-call, support |
| CLI commands, validation | `docs/developer-playbook.md` | Engineers |
| API contracts | `docs/reference/` | Engineers, integrators |
| Enums, error codes, patterns | `.github/knowledge/` | AI agents |
| Coding standards | `.github/instructions/` | AI agents, engineers |

### Style Rules

1. **Link, don't duplicate** — Reference existing docs instead of copying content
2. **One source of truth** — Each fact lives in exactly one file
3. **Mermaid for diagrams** — Use mermaid code blocks for all diagrams
4. **Tables for structured data** — Prefer tables over prose for enums, flags, mappings
5. **Code blocks for commands** — Always use fenced code blocks with language tag
6. **Cross-references** — Use relative paths: `[architecture](architecture.md)`

### When to Update

- New state machine state or transition → `docs/architecture.md`
- New callback or changed ordering → `docs/reference/callbacks.md`
- New TR-181 parameter → `docs/reference/tr181-matrix.md`
- New HAL function → `docs/reference/hal-api.md`
- New build flag → `.github/knowledge/reference-data.md`
- New failure pattern → `.github/knowledge/reference-data.md` + `docs/troubleshooting.md`
- New diagnostic command → `docs/developer-playbook.md`

### Quality Checklist

- [ ] No broken cross-references
- [ ] No duplicated content across files
- [ ] All code examples are correct and runnable
- [ ] Diagrams render in GitHub markdown preview
- [ ] New content linked from `docs/README.md` if top-level
