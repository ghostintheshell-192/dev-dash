import React, { useState } from 'react';
import { Folder, FileText, GitBranch, Search, Settings, ChevronRight, ChevronDown, ChevronUp, Zap, Terminal, BookOpen, Target, Plus, Trash2, ExternalLink, Layers, ClipboardList, Archive, X, Save, Edit3, PanelLeftClose, PanelLeft, PanelBottomClose, PanelBottom, Maximize2, Minimize2 } from 'lucide-react';

// Mock data (abbreviated)
const mockWorkspaces = [
  { id: 1, name: 'Coding', path: '/data/repos', type: 'coding', icon: '💻' },
  { id: 2, name: 'Writing', path: '/data/documenti/Vault@Racconti', type: 'writing', icon: '✍️' },
];

const mockProjects = {
  1: [
    { id: 'sa', name: 'sheet-atlas', path: '/data/repos/sheet-atlas', branch: 'develop', hasPersonal: true, hasDocs: true, language: 'csharp' },
    { id: 'gf', name: 'government-feed', path: '/data/repos/government-feed', branch: 'main', hasPersonal: false, hasDocs: true, language: 'csharp' },
  ],
  2: [
    { id: 'usv', name: 'Una storia vera', path: '/data/documenti/Vault@Racconti/Una storia vera', hasPersonal: false, hasDocs: false },
    { id: 'gdp', name: 'Giochi di potere', path: '/data/documenti/Vault@Racconti/Giochi di potere', hasPersonal: false, hasDocs: false },
  ]
};

const mockPersonalStructure = {
  'CURRENT-STATUS.md': { type: 'file', modified: '2h ago' },
  'INDEX.md': { type: 'file', modified: '10m ago' },
  'active': {
    type: 'folder',
    children: {
      'current-notes.md': { type: 'file' },
      'tech-debt': { type: 'folder', children: { 'cellmetadata-memory-waste.md': { type: 'file', priority: 'medium' }, 'json-persistence-proposal.md': { type: 'file', priority: 'medium' } } }
    }
  },
  'specs': {
    type: 'folder',
    children: {
      'planned': { type: 'folder', children: { 'column-filtering.md': { type: 'file' }, 'export-results.md': { type: 'file' } } },
      'backlog': { type: 'folder', children: { 'telemetry.md': { type: 'file' } } },
    }
  },
  'reference': {
    type: 'folder',
    children: {
      'decisions': { type: 'folder', children: { '001-error-handling.md': { type: 'file' }, '002-row-indexing.md': { type: 'file' } } },
    }
  },
};

const mockConfigs = [
  { name: 'Claude Global', path: '~/.claude/CLAUDE.md', icon: Zap },
  { name: 'Bootstrap Coding', path: '~/.claude/bootstrap-coding.md', icon: Terminal },
  { name: 'Workspace Rules', path: '/data/repos/CLAUDE.md', icon: Folder },
  { name: 'Claude Desktop', path: '~/.config/Claude/claude_desktop_config.json', icon: Settings },
];

const mockChatHistory = [
  { role: 'assistant', content: 'Ciao! Come posso aiutarti con sheet-atlas oggi?' },
  { role: 'user', content: 'vorrei lavorare sulla spec column-filtering' },
  { role: 'assistant', content: 'Perfetto! Ho letto la spec in `.personal/specs/planned/column-filtering.md`. Vuoi che rivediamo i requisiti insieme o preferisci iniziare direttamente con l\'implementazione?' },
];

export default function DevDash() {
  const [selectedWorkspace, setSelectedWorkspace] = useState(mockWorkspaces[0]);
  const [selectedProject, setSelectedProject] = useState(mockProjects[1][0]);
  const [activeTab, setActiveTab] = useState('personal');
  const [expandedFolders, setExpandedFolders] = useState(['active', 'specs', 'reference']);
  const [selectedFile, setSelectedFile] = useState(null);
  const [showSettings, setShowSettings] = useState(false);
  
  // Panel visibility
  const [showSidebar, setShowSidebar] = useState(true);
  const [showTerminal, setShowTerminal] = useState(true);
  const [terminalExpanded, setTerminalExpanded] = useState(false);
  const [showDocs, setShowDocs] = useState(true);

  const toggleFolder = (path) => {
    setExpandedFolders(prev => prev.includes(path) ? prev.filter(p => p !== path) : [...prev, path]);
  };

  const currentProjects = mockProjects[selectedWorkspace.id] || [];

  const PriorityBadge = ({ priority }) => {
    const colors = { high: 'bg-red-500/20 text-red-400', medium: 'bg-amber-500/20 text-amber-400', low: 'bg-slate-500/20 text-slate-400' };
    return <span className={`px-1.5 py-0.5 rounded text-xs ${colors[priority]}`}>{priority}</span>;
  };

  const FileTree = ({ structure, basePath = '', depth = 0 }) => {
    return Object.entries(structure).map(([name, item]) => {
      const fullPath = basePath ? `${basePath}/${name}` : name;
      const isFolder = item.type === 'folder';
      const isExpanded = expandedFolders.includes(fullPath);
      
      return (
        <div key={fullPath}>
          <button
            onClick={() => isFolder ? toggleFolder(fullPath) : setSelectedFile({ name, path: fullPath, ...item })}
            className={`w-full flex items-center gap-2 px-2 py-1 rounded text-left text-sm hover:bg-slate-700/50 ${selectedFile?.path === fullPath ? 'bg-amber-500/10 text-amber-400' : 'text-slate-300'}`}
            style={{ paddingLeft: `${depth * 12 + 8}px` }}
          >
            {isFolder ? (isExpanded ? <ChevronDown className="w-3 h-3 text-slate-500" /> : <ChevronRight className="w-3 h-3 text-slate-500" />) : <span className="w-3" />}
            {isFolder ? <Folder className="w-3.5 h-3.5 text-amber-400/70" /> : <FileText className="w-3.5 h-3.5 text-slate-500" />}
            <span className="truncate flex-1">{name}</span>
            {item.priority && <PriorityBadge priority={item.priority} />}
          </button>
          {isFolder && isExpanded && item.children && <FileTree structure={item.children} basePath={fullPath} depth={depth + 1} />}
        </div>
      );
    });
  };

  // Terminal Panel
  const TerminalPanel = () => (
    <div className={`border-t border-slate-700 bg-slate-900 flex flex-col ${terminalExpanded ? 'flex-1' : 'h-64'}`}>
      <div className="flex items-center justify-between px-3 py-2 border-b border-slate-700 bg-slate-800/50">
        <div className="flex items-center gap-2">
          <Terminal className="w-4 h-4 text-emerald-400" />
          <span className="text-sm font-medium">Claude Code</span>
          <span className="text-xs text-slate-500 font-mono">~/sheet-atlas</span>
        </div>
        <div className="flex items-center gap-1">
          <button onClick={() => setTerminalExpanded(!terminalExpanded)} className="p-1 hover:bg-slate-700 rounded" title={terminalExpanded ? "Riduci" : "Espandi"}>
            {terminalExpanded ? <Minimize2 className="w-4 h-4 text-slate-400" /> : <Maximize2 className="w-4 h-4 text-slate-400" />}
          </button>
          <button onClick={() => setShowTerminal(false)} className="p-1 hover:bg-slate-700 rounded" title="Chiudi terminale">
            <ChevronDown className="w-4 h-4 text-slate-400" />
          </button>
        </div>
      </div>
      <div className="flex-1 overflow-auto p-3 font-mono text-sm">
        {mockChatHistory.map((msg, i) => (
          <div key={i} className={`mb-3 ${msg.role === 'user' ? 'text-blue-300' : 'text-slate-300'}`}>
            <span className={`text-xs ${msg.role === 'user' ? 'text-blue-500' : 'text-emerald-500'}`}>
              {msg.role === 'user' ? '> ' : '🤖 '}
            </span>
            {msg.content}
          </div>
        ))}
        <div className="flex items-center gap-2 text-slate-400">
          <span className="text-emerald-400">❯</span>
          <span className="animate-pulse">_</span>
        </div>
      </div>
    </div>
  );

  // Settings Modal
  const SettingsModal = () => (
    <div className="fixed inset-0 bg-black/60 flex items-center justify-center z-50" onClick={() => setShowSettings(false)}>
      <div className="bg-slate-900 border border-slate-700 rounded-xl w-[600px] max-h-[80vh] overflow-hidden" onClick={e => e.stopPropagation()}>
        <div className="flex items-center justify-between p-4 border-b border-slate-700">
          <h2 className="font-semibold flex items-center gap-2"><Settings className="w-5 h-5" />Settings</h2>
          <button onClick={() => setShowSettings(false)} className="p-1 hover:bg-slate-700 rounded"><X className="w-5 h-5" /></button>
        </div>
        <div className="p-4 space-y-6 overflow-auto max-h-[60vh]">
          <div>
            <h3 className="text-sm font-medium text-slate-300 mb-3">Workspaces</h3>
            <div className="space-y-2">
              {mockWorkspaces.map(ws => (
                <div key={ws.id} className="flex items-center gap-3 bg-slate-800/50 p-3 rounded-lg">
                  <span className="text-xl">{ws.icon}</span>
                  <div className="flex-1">
                    <div className="font-medium text-sm">{ws.name}</div>
                    <div className="text-xs text-slate-500 font-mono">{ws.path}</div>
                  </div>
                  <button className="p-1 hover:bg-slate-700 rounded text-slate-400"><Edit3 className="w-4 h-4" /></button>
                  <button className="p-1 hover:bg-red-500/20 rounded text-red-400"><Trash2 className="w-4 h-4" /></button>
                </div>
              ))}
              <button className="w-full flex items-center justify-center gap-2 p-3 border border-dashed border-slate-600 rounded-lg text-slate-400 hover:border-slate-500">
                <Plus className="w-4 h-4" />Add Workspace
              </button>
            </div>
            <p className="text-xs text-slate-500 mt-2">Changes sync to ~/.claude/CLAUDE.md</p>
          </div>
          <div>
            <h3 className="text-sm font-medium text-slate-300 mb-3">Config Files</h3>
            <div className="grid grid-cols-2 gap-2">
              {mockConfigs.map((cfg, i) => (
                <button key={i} className="flex items-center gap-2 p-2 bg-slate-800/50 rounded-lg hover:bg-slate-700/50 text-left">
                  <cfg.icon className="w-4 h-4 text-slate-400" />
                  <div className="flex-1 min-w-0">
                    <div className="text-sm truncate">{cfg.name}</div>
                    <div className="text-xs text-slate-500 font-mono truncate">{cfg.path}</div>
                  </div>
                </button>
              ))}
            </div>
          </div>
        </div>
        <div className="p-4 border-t border-slate-700 flex justify-end gap-2">
          <button onClick={() => setShowSettings(false)} className="px-4 py-2 text-sm hover:bg-slate-700 rounded-lg">Cancel</button>
          <button className="px-4 py-2 text-sm bg-amber-600 hover:bg-amber-500 rounded-lg flex items-center gap-2"><Save className="w-4 h-4" />Save</button>
        </div>
      </div>
    </div>
  );

  return (
    <div className="h-screen bg-slate-950 text-slate-200 flex flex-col overflow-hidden">
      {showSettings && <SettingsModal />}
      
      {/* Top Bar */}
      <div className="h-10 bg-slate-900 border-b border-slate-800 flex items-center justify-between px-2 shrink-0">
        <div className="flex items-center gap-2">
          <button onClick={() => setShowSidebar(!showSidebar)} className={`p-1.5 rounded ${showSidebar ? 'bg-slate-700' : 'hover:bg-slate-800'}`} title="Toggle sidebar">
            {showSidebar ? <PanelLeftClose className="w-4 h-4" /> : <PanelLeft className="w-4 h-4" />}
          </button>
          <div className="w-px h-5 bg-slate-700" />
          <Zap className="w-4 h-4 text-amber-400" />
          <span className="font-medium text-sm">DevDash</span>
        </div>
        
        <div className="flex items-center gap-2">
          <span className="text-xs text-slate-500">{selectedProject?.name}</span>
          <div className="w-px h-5 bg-slate-700" />
          <button onClick={() => setShowDocs(!showDocs)} className={`p-1.5 rounded ${showDocs ? 'bg-slate-700' : 'hover:bg-slate-800'}`} title="Toggle docs">
            <BookOpen className="w-4 h-4" />
          </button>
          <button onClick={() => setShowTerminal(!showTerminal)} className={`p-1.5 rounded ${showTerminal ? 'bg-emerald-600/30 text-emerald-400' : 'hover:bg-slate-800'}`} title="Toggle terminal">
            <Terminal className="w-4 h-4" />
          </button>
          <div className="w-px h-5 bg-slate-700" />
          <button onClick={() => setShowSettings(true)} className="p-1.5 hover:bg-slate-800 rounded">
            <Settings className="w-4 h-4 text-slate-400" />
          </button>
        </div>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Sidebar */}
        {showSidebar && (
          <div className="w-64 bg-slate-900 border-r border-slate-800 flex flex-col shrink-0">
            {/* Workspace Selector */}
            <div className="p-2 border-b border-slate-800">
              <div className="flex gap-1">
                {mockWorkspaces.map(ws => (
                  <button
                    key={ws.id}
                    onClick={() => { setSelectedWorkspace(ws); setSelectedProject(mockProjects[ws.id]?.[0]); }}
                    className={`flex-1 flex items-center justify-center gap-1.5 px-2 py-1.5 rounded text-xs transition-all ${selectedWorkspace.id === ws.id ? 'bg-amber-500/20 text-amber-400' : 'bg-slate-800 hover:bg-slate-700'}`}
                  >
                    <span>{ws.icon}</span><span>{ws.name}</span>
                  </button>
                ))}
              </div>
            </div>

            {/* Projects */}
            <div className="flex-1 overflow-auto p-2">
              <div className="text-xs text-slate-500 uppercase tracking-wider mb-2 px-2">Projects</div>
              {currentProjects.map(project => (
                <div
                  key={project.id}
                  onClick={() => setSelectedProject(project)}
                  className={`mb-1 p-2 rounded-lg cursor-pointer transition-all ${selectedProject?.id === project.id ? 'bg-amber-500/10 border border-amber-500/30' : 'hover:bg-slate-800 border border-transparent'}`}
                >
                  <div className="flex items-center justify-between">
                    <span className="font-medium text-sm">{project.name}</span>
                    <div className="flex items-center gap-1">
                      {project.hasPersonal && <Target className="w-3 h-3 text-amber-400" />}
                      {project.hasDocs && <BookOpen className="w-3 h-3 text-blue-400" />}
                    </div>
                  </div>
                  {project.branch && (
                    <div className="flex items-center gap-1.5 mt-1 text-xs text-slate-500">
                      <GitBranch className="w-3 h-3" />{project.branch}
                    </div>
                  )}
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Main Area */}
        <div className="flex-1 flex flex-col overflow-hidden">
          {/* Docs Panel */}
          {showDocs && !terminalExpanded && (
            <div className="flex-1 flex flex-col overflow-hidden">
              {/* Tabs */}
              <div className="flex gap-1 px-3 pt-2 bg-slate-900/50 border-b border-slate-800 shrink-0">
                {[
                  { id: 'personal', label: '.personal', icon: Target },
                  { id: 'docs', label: 'docs', icon: BookOpen },
                  { id: 'issues', label: 'Issues', icon: ClipboardList },
                  { id: 'adr', label: 'ADR', icon: Archive },
                  { id: 'config', label: 'Config', icon: Layers },
                ].map(tab => (
                  <button
                    key={tab.id}
                    onClick={() => setActiveTab(tab.id)}
                    className={`flex items-center gap-1.5 px-3 py-1.5 rounded-t text-xs transition-colors ${activeTab === tab.id ? 'bg-slate-800 text-amber-400' : 'text-slate-400 hover:text-slate-200'}`}
                  >
                    <tab.icon className="w-3.5 h-3.5" />{tab.label}
                  </button>
                ))}
              </div>

              {/* Content */}
              <div className="flex-1 flex overflow-hidden bg-slate-800/30">
                {activeTab === 'config' ? (
                  <div className="flex-1 p-4 overflow-auto">
                    <div className="grid grid-cols-2 gap-2">
                      {mockConfigs.map((cfg, i) => (
                        <button key={i} className="flex items-center gap-2 p-3 bg-slate-900/50 border border-slate-700 rounded-lg hover:border-slate-600 text-left">
                          <cfg.icon className="w-4 h-4 text-amber-400" />
                          <div className="flex-1 min-w-0">
                            <div className="text-sm">{cfg.name}</div>
                            <div className="text-xs text-slate-500 font-mono truncate">{cfg.path}</div>
                          </div>
                        </button>
                      ))}
                    </div>
                  </div>
                ) : (
                  <>
                    <div className="w-64 border-r border-slate-700 overflow-auto shrink-0">
                      <div className="p-2">
                        <FileTree structure={mockPersonalStructure} />
                      </div>
                    </div>
                    <div className="flex-1 p-4 overflow-auto">
                      {selectedFile ? (
                        <div className="h-full flex flex-col">
                          <div className="flex items-center justify-between mb-3">
                            <h3 className="font-medium text-sm flex items-center gap-2">
                              <FileText className="w-4 h-4 text-slate-400" />{selectedFile.name}
                            </h3>
                            <button className="text-xs bg-slate-700 hover:bg-slate-600 px-2 py-1 rounded flex items-center gap-1">
                              <Edit3 className="w-3 h-3" />Edit
                            </button>
                          </div>
                          <div className="flex-1 bg-slate-900/50 border border-slate-700 rounded-lg p-4 font-mono text-sm text-slate-300 overflow-auto">
                            <pre className="whitespace-pre-wrap">{`# ${selectedFile.name.replace('.md', '')}\n\nPreview del contenuto markdown.\n\n---\n\n*Path: ${selectedFile.path}*`}</pre>
                          </div>
                        </div>
                      ) : (
                        <div className="h-full flex items-center justify-center text-slate-500">
                          <div className="text-center">
                            <FileText className="w-10 h-10 mx-auto mb-2 opacity-30" />
                            <p className="text-sm">Seleziona un file</p>
                          </div>
                        </div>
                      )}
                    </div>
                  </>
                )}
              </div>
            </div>
          )}

          {/* Terminal */}
          {showTerminal && <TerminalPanel />}
          
          {/* Terminal Toggle when closed */}
          {!showTerminal && (
            <button 
              onClick={() => setShowTerminal(true)}
              className="h-8 bg-slate-900 border-t border-slate-700 flex items-center justify-center gap-2 text-xs text-slate-400 hover:text-slate-200 hover:bg-slate-800 shrink-0"
            >
              <ChevronUp className="w-4 h-4" />
              <Terminal className="w-4 h-4" />
              Open Claude Code
            </button>
          )}
        </div>
      </div>
    </div>
  );
}
