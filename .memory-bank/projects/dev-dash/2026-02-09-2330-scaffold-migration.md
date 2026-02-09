## 2026-02-09 - Migrazione scaffold da workspace-level a project-level

**Done**:
- Migrato scaffold dal pattern workspace-level (`.rules/`) al pattern ufficiale Claude Code project-level (`.claude/rules/`)
- Creata nuova struttura modulare in `rsrc/workspace-scaffold/.claude/rules/`: overview.md, coding-standards.md, principles.md, preflight-checks.md, workflow.md
- Aggiornato `ScaffoldService` con `ApplyProjectScaffoldAsync()` per inizializzazione project-level
- Backup del vecchio sistema in `rsrc/legacy-workspace-rules/` (sia scaffold-rules che workspace-root-rules)
- Implementata UI di inizializzazione progetto single-project (semplificata da approccio multi-project)
- Creati `ProjectInitializationViewModel` e `ProjectInitializationPanel.axaml`
- Aggiunto tab "Projects" in MainWindow per gestione inizializzazione
- Aggiornato `Project` model con proprietà `IsConfigured`, `HasClaudeDir`, `HasRules`
- Aggiornato `WorkspaceService.GetProjectsAsync()` per popolare stato configurazione progetti
- Fix: pulsante Initialize ora si abilita correttamente quando si seleziona un progetto non inizializzato
- Commit locale completato per lo scaffolding (83 file, 4516+ righe)

**Next**:
- Committare i file UI rimanenti (ViewModels, Views per inizializzazione progetti)
- Testare l'inizializzazione su un progetto reale per verificare che i file `.claude/rules/` vengano creati correttamente
- Eventualmente risolvere l'artefatto visivo residuo sopra il pulsante Initialize (uno dei due è stato rimosso, ma ne rimane ancora uno)
- Considerare di aggiungere feedback visivo durante il processo di inizializzazione (spinner o progress indicator)

**Notes**:
- Il progetto ora segue il pattern ufficiale: ogni progetto è self-contained con `.claude/rules/` modular
- L'embedded resources configuration nel `.csproj` include già i nuovi file `.claude/rules/*.md`
- Il pulsante era disabilitato perché mancava `OnCurrentProjectChanged()` per notificare il comando
- Rimangono warning di formatting in alcuni file pre-esistenti (IFileSystemService, ViewLocator, DocumentTabViewModel) - non bloccanti
- L'utente ha apprezzato la semplificazione da multi-project a single-project usando la sidebar esistente
