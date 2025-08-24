import React from 'react';

export interface AIHelpPanelProps {
  onAsk: (text: string) => void;
  onTroubleshoot: () => void;
  onInsights: () => void;
  onFeeOptimize: () => void;
  onLanguage: () => void;
  messages: { role: 'user' | 'assistant'; content: string }[];
}

export const AIHelpPanel: React.FC<AIHelpPanelProps> = ({
  onAsk, onTroubleshoot, onInsights, onFeeOptimize, onLanguage, messages,
}) => {
  const [input, setInput] = React.useState('');
  return (
    <div style={{ display: 'grid', gridTemplateColumns: '280px 1fr', gap: 16, height: '100%' }}>
      <aside style={{ borderRight: '1px solid #e5e7eb', padding: 12 }}>
        <h3>AI Tools</h3>
        <div style={{ display: 'grid', gap: 8 }}>
          <button onClick={onInsights}>📊 Insights</button>
          <button onClick={onTroubleshoot}>🛠 Troubleshooter</button>
          <button onClick={onFeeOptimize}>⚡ Fee Optimizer</button>
          <button onClick={onLanguage}>🌐 Multilingual</button>
        </div>
      </aside>
      <section style={{ padding: 12, display: 'flex', flexDirection: 'column', height: '100%' }}>
        <h3>GPT Assistant</h3>
        <div style={{ flex: 1, overflow: 'auto', border: '1px solid #e5e7eb', borderRadius: 8, padding: 12 }}>
          {messages.map((m, i) => (
            <div key={i} style={{ marginBottom: 8 }}>
              <strong>{m.role === 'user' ? 'You' : 'Assistant'}:</strong> {m.content}
            </div>
          ))}
        </div>
        <div style={{ marginTop: 8, display: 'flex', gap: 8 }}>
          <input value={input} onChange={e => setInput(e.target.value)} placeholder="Ask anything about Shahcoin…" style={{ flex: 1, padding: 8 }} />
          <button onClick={() => { onAsk(input); setInput(''); }}>Send</button>
        </div>
      </section>
    </div>
  );
};

export default AIHelpPanel;


