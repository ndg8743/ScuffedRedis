'use client';

import React, { useState, useRef, useEffect } from 'react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { X, Terminal, Send, Copy, BookOpen, Clock } from 'lucide-react';
import { SERVER_URL } from '@/lib/config';

interface CommandResult {
  command: string;
  result: any;
  error?: string;
  executionTime: number;
  timestamp: Date;
}

interface CommandExample {
  category: string;
  commands: {
    cmd: string;
    description: string;
    example?: string;
  }[];
}

const COMMAND_EXAMPLES: CommandExample[] = [
  {
    category: 'Basic Operations',
    commands: [
      { cmd: 'PING', description: 'Test server connectivity', example: 'PING' },
      { cmd: 'SET', description: 'Set a key-value pair', example: 'SET mykey "Hello World"' },
      { cmd: 'GET', description: 'Get value by key', example: 'GET mykey' },
      { cmd: 'DEL', description: 'Delete a key', example: 'DEL mykey' },
      { cmd: 'EXISTS', description: 'Check if key exists', example: 'EXISTS mykey' }
    ]
  },
  {
    category: 'Server Info',
    commands: [
      { cmd: 'INFO', description: 'Get server statistics', example: 'INFO' },
      { cmd: 'DBSIZE', description: 'Count total keys', example: 'DBSIZE' },
      { cmd: 'FLUSHDB', description: 'Clear all keys (careful!)', example: 'FLUSHDB' }
    ]
  },
  {
    category: 'Pattern Matching',
    commands: [
      { cmd: 'KEYS', description: 'Find keys by pattern', example: 'KEYS item:*' },
      { cmd: 'KEYS', description: 'Match with wildcards', example: 'KEYS user:?123' }
    ]
  },
  {
    category: 'Sorted Sets',
    commands: [
      { cmd: 'ZADD', description: 'Add to sorted set', example: 'ZADD leaderboard 100 alice 95 bob' },
      { cmd: 'ZRANGE', description: 'Get range by rank', example: 'ZRANGE leaderboard 0 -1 WITHSCORES' },
      { cmd: 'ZRANK', description: 'Get member rank', example: 'ZRANK leaderboard alice' },
      { cmd: 'ZSCORE', description: 'Get member score', example: 'ZSCORE leaderboard alice' }
    ]
  }
];

interface CommandPlaygroundProps {
  isOpen: boolean;
  onClose: () => void;
}

export function CommandPlayground({ isOpen, onClose }: CommandPlaygroundProps) {
  const [command, setCommand] = useState('');
  const [history, setHistory] = useState<CommandResult[]>([]);
  const [historyIndex, setHistoryIndex] = useState(-1);
  const [loading, setLoading] = useState(false);
  const [showExamples, setShowExamples] = useState(true);
  const inputRef = useRef<HTMLInputElement>(null);
  const historyEndRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (isOpen && inputRef.current) {
      inputRef.current.focus();
    }
  }, [isOpen]);

  useEffect(() => {
    historyEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [history]);

  const executeCommand = async (cmd: string = command.trim()) => {
    if (!cmd) return;

    setLoading(true);
    const startTime = Date.now();

    try {
      const response = await fetch(`${SERVER_URL}/execute`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ command: cmd }),
      });

      const data = await response.json();
      const executionTime = Date.now() - startTime;

      const result: CommandResult = {
        command: cmd,
        result: data.result,
        error: data.error,
        executionTime,
        timestamp: new Date(),
      };

      setHistory(prev => [...prev, result]);
      setCommand('');
      setHistoryIndex(-1);
      setShowExamples(false);
    } catch (error) {
      const executionTime = Date.now() - startTime;
      setHistory(prev => [...prev, {
        command: cmd,
        result: null,
        error: 'Failed to execute command: ' + (error as Error).message,
        executionTime,
        timestamp: new Date(),
      }]);
    } finally {
      setLoading(false);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      executeCommand();
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      if (historyIndex < history.length - 1) {
        const newIndex = historyIndex + 1;
        setHistoryIndex(newIndex);
        setCommand(history[history.length - 1 - newIndex].command);
      }
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      if (historyIndex > 0) {
        const newIndex = historyIndex - 1;
        setHistoryIndex(newIndex);
        setCommand(history[history.length - 1 - newIndex].command);
      } else if (historyIndex === 0) {
        setHistoryIndex(-1);
        setCommand('');
      }
    }
  };

  const copyToClipboard = (text: string) => {
    navigator.clipboard.writeText(text);
  };

  const formatResult = (result: any): string => {
    if (result === null) return '(nil)';
    if (typeof result === 'object') {
      return JSON.stringify(result, null, 2);
    }
    return String(result);
  };

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/50 backdrop-blur-sm">
      <Card className="w-[90%] max-w-4xl h-[80vh] flex flex-col bg-gray-900 border-gray-700 text-gray-100">
        <CardHeader className="flex flex-row items-center justify-between pb-3">
          <div className="flex items-center gap-2">
            <Terminal className="w-5 h-5" />
            <CardTitle className="text-xl">Redis Command Playground</CardTitle>
            <Badge variant="outline" className="ml-2">
              Full Server - Port 6380
            </Badge>
          </div>
          <Button
            variant="ghost"
            size="icon"
            onClick={onClose}
            className="hover:bg-gray-800"
          >
            <X className="w-4 h-4" />
          </Button>
        </CardHeader>

        <CardContent className="flex-1 overflow-hidden p-0 flex">
          <div className="flex-1 flex flex-col">
            {/* Terminal Output */}
            <div className="flex-1 overflow-y-auto p-4 font-mono text-sm bg-black/50">
              {showExamples && history.length === 0 && (
                <div className="text-gray-400 mb-4">
                  <div className="mb-2">Welcome to the Redis Command Playground!</div>
                  <div className="mb-2">Type commands below or click an example to get started.</div>
                  <div>Use ↑↓ arrows to navigate command history.</div>
                </div>
              )}

              {history.map((item, index) => (
                <div key={index} className="mb-4">
                  <div className="flex items-start gap-2">
                    <span className="text-green-400">redis&gt;</span>
                    <span className="text-gray-300 flex-1">{item.command}</span>
                    <Badge
                      variant="outline"
                      className="text-xs"
                    >
                      <Clock className="w-3 h-3 mr-1" />
                      {item.executionTime}ms
                    </Badge>
                    <Button
                      variant="ghost"
                      size="icon"
                      className="w-6 h-6"
                      onClick={() => copyToClipboard(item.command)}
                    >
                      <Copy className="w-3 h-3" />
                    </Button>
                  </div>
                  <div className="ml-8 mt-1">
                    {item.error ? (
                      <span className="text-red-400">(error) {item.error}</span>
                    ) : (
                      <pre className="text-yellow-300 whitespace-pre-wrap">
                        {formatResult(item.result)}
                      </pre>
                    )}
                  </div>
                </div>
              ))}
              <div ref={historyEndRef} />
            </div>

            {/* Command Input */}
            <div className="border-t border-gray-700 p-4">
              <div className="flex gap-2">
                <span className="text-green-400 font-mono">redis&gt;</span>
                <input
                  ref={inputRef}
                  type="text"
                  value={command}
                  onChange={(e) => setCommand(e.target.value)}
                  onKeyDown={handleKeyDown}
                  disabled={loading}
                  placeholder="Enter Redis command..."
                  className="flex-1 bg-transparent outline-none font-mono text-sm text-gray-100 placeholder-gray-500"
                  autoComplete="off"
                  spellCheck={false}
                />
                <Button
                  size="sm"
                  onClick={() => executeCommand()}
                  disabled={loading || !command.trim()}
                  className="bg-blue-600 hover:bg-blue-700"
                >
                  <Send className="w-4 h-4" />
                </Button>
              </div>
            </div>
          </div>

          {/* Examples Panel */}
          {showExamples && (
            <div className="w-80 border-l border-gray-700 p-4 overflow-y-auto">
              <div className="flex items-center gap-2 mb-4">
                <BookOpen className="w-4 h-4" />
                <h3 className="font-semibold">Command Examples</h3>
              </div>

              {COMMAND_EXAMPLES.map((category) => (
                <div key={category.category} className="mb-6">
                  <h4 className="text-sm font-medium text-gray-400 mb-2">
                    {category.category}
                  </h4>
                  <div className="space-y-1">
                    {category.commands.map((cmd, idx) => (
                      <button
                        key={idx}
                        onClick={() => {
                          if (cmd.example) {
                            setCommand(cmd.example);
                            inputRef.current?.focus();
                          }
                        }}
                        className="w-full text-left p-2 rounded hover:bg-gray-800 transition-colors group"
                      >
                        <div className="font-mono text-sm text-blue-400 group-hover:text-blue-300">
                          {cmd.cmd}
                        </div>
                        <div className="text-xs text-gray-500">
                          {cmd.description}
                        </div>
                        {cmd.example && (
                          <div className="text-xs font-mono text-gray-600 mt-1">
                            {cmd.example}
                          </div>
                        )}
                      </button>
                    ))}
                  </div>
                </div>
              ))}
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}