# ADR-016: Versionamento git gestito degli scaffold

**Data**: 2026-06-28
**Status**: Accepted
**Impact**: high
**Sommario**: DevDash versiona gli scaffold con git in modo gestito — un repo per scaffold, commit "sotto il cofano" con diff mostrati, git come unica storia degli scaffold — astenendosi sui symlink/repo esterni (dogfooding di ADR-013); apre all'integrazione progressiva di git nella codebase (diff-compute, merge).

## Contesto

`feature-scaffold-management` (US-3) recita: *"DevDash non versiona gli scaffold;
se l'utente li tiene in git, i commit li fa lui con strumenti standard."* È la
versione "astieniti" dello stesso "no magic" che ADR-015 ha ribaltato per i
placeholder: DevDash che si tira indietro e lascia il lavoro manuale.

Materialmente quell'astensione è scomoda e incoerente:

- Uno scaffold senza storia gestita costringe l'utente a ricordarsi di committare
  a mano, con git, fuori dall'app.
- DevDash è un **gestore visuale di configurazioni**: pretendere l'uso della CLI
  git a mano contraddice la sua premessa — esattamente come pretendere l'edit
  manuale del manifest (ADR-015).

git è lo standard de facto del versionamento; non si introduce alcuna astrazione
su VCS alternativi (tfvc, mercurial): git è trattato come **dato dell'ambiente**.

## Decisione

1. **Un repo git per scaffold.** Ogni scaffold in `~/.devdash/scaffolds/<name>/`
   è un repo git indipendente con la propria storia — coerente con la loro
   molteplicità (`feature-scaffold-management` US-4 / ADR-015 §6).

2. **git è l'unica storia degli scaffold.** Nessun sistema di snapshot parallelo
   per gli scaffold; gli snapshot di `feature-snapshot-history` restano per i
   **progetti**. Un solo meccanismo di history per ciascun dominio.

3. **Commit gestiti dall'app, diff sempre mostrati.** Ogni modifica a uno
   scaffold (promote, edit del manifest, …) → DevDash mostra il diff → l'utente
   conferma → l'app esegue `git add` + `commit` con messaggio generato. L'utente
   non tocca la CLI. `git init` è gestito dall'app; l'**identità del committer è
   quella della config git dell'utente** (locale o globale, con la precedenza
   che git già risolve — nessuna identità dedicata DevDash). Se git non è
   configurato, l'app lo segnala e rimanda alla configurazione di git, senza
   inventare un'identità di ripiego.

4. **Astensione su symlink / repo esterni.** Se lo scaffold è un symlink (caso
   dogfooding di ADR-013: `dev-dash-standard` → `rsrc/project-scaffold/`, dentro
   il repo dev-dash) o è già tracciato da un repo git esterno, DevDash **non
   auto-committa**: il versionamento spetta al repo che lo contiene. In sviluppo
   ciò coincide con la disciplina git dell'utente (branch → merge in `develop`),
   che è proprio ciò che si vuole lì.

5. **Marker default fuori dal versionamento.** Il `.default-scaffold` è una
   preferenza locale dell'utente, non contenuto da propagare → **gitignored**
   nello scaffold-repo.

6. **Integrazione progressiva di git.** Avere git nello stack abilita usi
   incrementali oltre il versioning: compute del diff via `git diff` (post-MVP),
   three-way merge dei conflitti d'apply via `git merge-file` (vedi ADR-015 OQ
   sulla mappa di risoluzione). Direzione esplicita: integrare git sempre più
   profondamente, a poco a poco.

7. **Portabilità.** L'implementazione di riferimento è lo *shelling* verso la
   `git` CLI — la via più cross-platform: la CLI è identica su Linux e Windows
   (target a termine, ADR-011), i path restano gestiti via `std::filesystem`.

## Rationale

- **Coerenza con ADR-015.** "No magic" significa *mostrare*, non *astenersi*. I
  commit gestiti con diff mostrati realizzano la trasparenza meglio
  dell'astensione, che è la versione davvero opaca (storia che dipende dal fatto
  che l'utente si ricordi di farla).
- **Coerenza di prodotto.** Un gestore visuale che richiede la git CLI a mano si
  auto-contraddice — stesso argomento del manifest GUI-only.
- **Un'unica storia per dominio.** git per gli scaffold, snapshot per i progetti:
  evita due meccanismi di history paralleli sullo stesso artefatto.
- **L'astensione non è un workaround.** Sul symlink-dev è esattamente il flusso
  che si vuole in sviluppo (dogfooding ADR-013).
- **git come ruota già fatta.** diff e merge battle-tested, gratis: una
  dipendenza che paga su più fronti (versioning, diff-compute, merge).

## Conseguenze

### Pro

- Storia robusta e standard per gli scaffold (diff, log, revert) senza costruirla.
- Coerenza filosofica e di prodotto con il resto delle decisioni.
- git riusato per diff-compute e merge → meno codice proprietario nel tempo.
- Piano d'implementazione più portabile (Windows quasi gratis).

### Contro

- Dipendenza da git a runtime.
- Identità del committer e `git init` automatici da gestire.
- Il caso speciale symlink / repo-esterno va rilevato e gestito.
- Conoscenza di git dentro l'app (shelling, parsing dell'output).

## Open Questions

- **Rilevamento "già in un repo git"**: come rilevarlo (`git rev-parse`) e cosa
  fare esattamente — astensione totale, o commit nel repo esistente mostrando il
  diff?
- **Granularità dei commit automatici**: un commit per apply/promote, o più fine?
- **Schema dei messaggi di commit** generati.

## Related

- ADR-013 — symlink in dev / copia per gli utenti: origine del caso "astensione".
- ADR-015 — templating: il `promote` che genera i commit; OQ mappa di risoluzione.
- `feature-scaffold-management` (US-3) — la frase "i commit li fa lui" che questo
  ADR rivede.
- `feature-snapshot-history` — snapshot per i progetti (storia distinta da quella
  git degli scaffold).
- ADR-011 — distribuzione: git come dipendenza di runtime, target Windows.
