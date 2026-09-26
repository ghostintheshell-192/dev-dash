"""Shared model for the code-graph extraction experiment (libclang vs tree-sitter)."""
from dataclasses import dataclass, field


@dataclass
class Member:
    name: str
    type: str          # as spelled / resolved by the extractor
    access: str        # public / protected / private
    is_method: bool


@dataclass
class ClassInfo:
    name: str          # qualified when the extractor can tell (ns::Name)
    file: str
    kind: str          # class / struct
    bases: list = field(default_factory=list)
    members: list = field(default_factory=list)


# Relation kinds, UML-ish:
#   inherits   A -> B   (A derives from B)
#   composes   A -> B   (A holds B by value / unique_ptr / optional / vector<B>)
#   aggregates A -> B   (A holds B& or B*)
#   depends    A -> B   (B appears in a method signature of A, not as a member)
@dataclass(frozen=True)
class Relation:
    src: str
    dst: str
    kind: str


def short(name: str) -> str:
    return name.split("::")[-1]
