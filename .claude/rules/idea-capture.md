# Idea Capture Rule

Convenzione per catturare idee tangenziali emerse in conversazione senza interrompere il workflow corrente.

## Quando attivare

**Trigger espliciti dell'utente** (frasi tipiche, in italiano):

- *"facciamo una nota e andiamo avanti"*
- *"salviamo per dopo"*
- *"non adesso, parcheggiamo"*
- *"questa è interessante ma fuori scope"*
- *"prendi nota e proseguiamo"*

**Trigger proattivo da Claude**: quando percepisci un thread tangenziale che ha dignità propria (merita attenzione futura) ma rischia di derailare il main work, **chiedi** *"vuoi che la noti come idea da esplorare?"* invece di seguire la tangente.

## Come catturare

1. **Crea un file** in `.memory-bank/ideas/` con pattern filename: `YYYY-MM-DD-<short-slug>.md` (data odierna + slug breve descrittivo)

2. **Frontmatter** richiesto:

   ```yaml
   ---
   captured: YYYY-MM-DD
   status: open
   context: "breve descrizione di dove è emersa l'idea (workstream, branch, file)"
   tags: [tag1, tag2]
   ---
   ```

3. **Body** (corto, focalizzato — NON è una spec):
   - Cos'è l'idea
   - Perché merita attenzione futura
   - Qual è il next-step minimo se mai si riprende

4. **Riprendi il thread principale** — la cattura deve essere veloce, non un detour.

## Stati ammessi (status)

- `open` — cattura fresca
- `parked` — esplicitamente differita per dopo
- `promoted-to-spec` / `promoted-to-tech-debt` / `promoted-to-adr` / `promoted-to-skill` — promossa a un artefatto durevole, link nel campo `promoted_to`
- `dropped` — decisa come non perseguibile

## Promote workflow

Quando un'idea matura abbastanza da diventare uno spec o altra struttura durevole:

1. Crea il nuovo artefatto nella sua location naturale (es. `.development/specs/planned/feature-X.md`)
2. **Aggiorna l'idea note** — NON cancellarla. Modifica frontmatter:
   ```yaml
   status: promoted-to-spec
   promoted_to: ../../.development/specs/planned/feature-X.md
   promoted_at: YYYY-MM-DD
   ```
3. La ground truth è il nuovo artefatto da quel momento. L'idea note resta come record di provenienza.

## Vedi anche

- `.memory-bank/ideas/README.md` — convenzione completa + bootstrap
- Resource model sezione G (Tangential idea note è una risorsa first-class)
