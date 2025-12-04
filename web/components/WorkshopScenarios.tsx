'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Separator } from '@/components/ui/separator';
import {
  PlayCircle,
  PauseCircle,
  RotateCcw,
  CheckCircle,
  AlertCircle,
  Clock,
  TrendingUp,
  Database,
  Zap,
  Users,
  Trophy,
  Timer,
  Hash
} from 'lucide-react';
import { SERVER_URL } from '@/lib/config';
import { useAppStore } from '@/lib/state';

interface Scenario {
  id: string;
  title: string;
  duration: string;
  difficulty: 'Beginner' | 'Intermediate' | 'Advanced';
  icon: React.ReactNode;
  description: string;
  objectives: string[];
  steps: ScenarioStep[];
}

interface ScenarioStep {
  instruction: string;
  command?: string;
  expectedResult?: string;
  explanation: string;
}

const SCENARIOS: Scenario[] = [
  {
    id: 'cold-vs-warm',
    title: 'Cold vs Warm Cache',
    duration: '10 min',
    difficulty: 'Beginner',
    icon: <Zap className="w-5 h-5" />,
    description: 'Understand the dramatic performance difference between cold and warm caches',
    objectives: [
      'Observe cache miss penalties',
      'See hit ratio improvement after warming',
      'Understand cache-aside pattern'
    ],
    steps: [
      {
        instruction: 'Clear the cache to start fresh',
        command: 'FLUSHDB',
        expectedResult: 'OK',
        explanation: 'Starting with an empty cache (cold cache)'
      },
      {
        instruction: 'Watch the visualization - notice all red flashes (misses)',
        explanation: 'Each red flash represents a slow database query (600-1200ms)'
      },
      {
        instruction: 'Check the current hit ratio',
        command: 'INFO',
        explanation: 'Should be 0% or very low - all requests are missing'
      },
      {
        instruction: 'Click "Warm Up" button to populate cache',
        explanation: 'This simulates pre-loading frequently accessed data'
      },
      {
        instruction: 'Observe the green flashes increase',
        explanation: 'Green means cache hits - fast responses (<10ms)'
      },
      {
        instruction: 'Check hit ratio again after 30 seconds',
        command: 'INFO',
        expectedResult: 'Hit ratio > 80%',
        explanation: 'Warm cache dramatically improves performance!'
      }
    ]
  },
  {
    id: 'ttl-expiration',
    title: 'TTL and Expiration',
    duration: '15 min',
    difficulty: 'Intermediate',
    icon: <Timer className="w-5 h-5" />,
    description: 'Learn how Redis automatically expires keys to prevent stale data',
    objectives: [
      'Set keys with different TTLs',
      'Observe automatic expiration',
      'Understand memory management'
    ],
    steps: [
      {
        instruction: 'Set a key with 30 second TTL',
        command: 'SET session:user123 "active" EX 30',
        expectedResult: 'OK',
        explanation: 'Key will automatically expire after 30 seconds'
      },
      {
        instruction: 'Verify the key exists',
        command: 'GET session:user123',
        expectedResult: 'active',
        explanation: 'Key is still valid within TTL window'
      },
      {
        instruction: 'Check remaining TTL',
        command: 'TTL session:user123',
        explanation: 'Shows seconds until expiration (or -2 if expired)'
      },
      {
        instruction: 'Set multiple keys with different TTLs',
        command: 'SET cache:short "data" EX 10',
        explanation: 'Different TTLs for different data types'
      },
      {
        instruction: 'Wait for expiration and check keys',
        command: 'KEYS session:*',
        explanation: 'Expired keys are automatically removed'
      },
      {
        instruction: 'Check memory usage',
        command: 'INFO',
        explanation: 'TTL helps manage memory by removing old data'
      }
    ]
  },
  {
    id: 'leaderboard',
    title: 'Building a Leaderboard',
    duration: '20 min',
    difficulty: 'Advanced',
    icon: <Trophy className="w-5 h-5" />,
    description: 'Implement a real-time game leaderboard using sorted sets',
    objectives: [
      'Use sorted sets for ranking',
      'Query top players efficiently',
      'Update scores in real-time'
    ],
    steps: [
      {
        instruction: 'Create a leaderboard with initial scores',
        command: 'ZADD game:leaderboard 1000 "Alice" 950 "Bob" 900 "Charlie"',
        expectedResult: '3',
        explanation: 'Sorted set automatically maintains score order'
      },
      {
        instruction: 'Get top 3 players',
        command: 'ZRANGE game:leaderboard 0 2 WITHSCORES',
        expectedResult: '["Charlie", "900", "Bob", "950", "Alice", "1000"]',
        explanation: 'Returns players sorted by score (ascending by default)'
      },
      {
        instruction: 'Get top players (descending)',
        command: 'ZREVRANGE game:leaderboard 0 2 WITHSCORES',
        expectedResult: '["Alice", "1000", "Bob", "950", "Charlie", "900"]',
        explanation: 'ZREVRANGE for high-to-low ranking'
      },
      {
        instruction: 'Update a player score',
        command: 'ZADD game:leaderboard 1100 "Bob"',
        explanation: 'Bob takes the lead! Sorted set reorders automatically'
      },
      {
        instruction: 'Get player rank',
        command: 'ZREVRANK game:leaderboard "Bob"',
        expectedResult: '0',
        explanation: 'Bob is now rank 0 (first place)'
      },
      {
        instruction: 'Add more players and check performance',
        command: 'ZADD game:leaderboard 850 "David" 920 "Eve" 1050 "Frank"',
        explanation: 'O(log n) operations scale efficiently'
      }
    ]
  },
  {
    id: 'pattern-matching',
    title: 'Pattern Matching & Search',
    duration: '15 min',
    difficulty: 'Intermediate',
    icon: <Hash className="w-5 h-5" />,
    description: 'Master pattern matching for efficient key discovery',
    objectives: [
      'Use wildcards effectively',
      'Understand performance implications',
      'Organize keys with namespacing'
    ],
    steps: [
      {
        instruction: 'Set up namespaced keys',
        command: 'SET user:1001:name "Alice"',
        explanation: 'Use colons for logical namespacing'
      },
      {
        instruction: 'Add more user data',
        command: 'SET user:1001:email "alice@example.com"',
        explanation: 'Related data under same namespace'
      },
      {
        instruction: 'Find all user 1001 keys',
        command: 'KEYS user:1001:*',
        expectedResult: '["user:1001:name", "user:1001:email"]',
        explanation: 'Asterisk matches any characters'
      },
      {
        instruction: 'Add session keys',
        command: 'SET session:abc123 "user:1001"',
        explanation: 'Different namespace for sessions'
      },
      {
        instruction: 'Find all session keys',
        command: 'KEYS session:*',
        explanation: 'Pattern matching across namespace'
      },
      {
        instruction: 'Use ? for single character',
        command: 'KEYS user:100?:name',
        explanation: 'Matches user:1001:name, user:1002:name, etc.'
      },
      {
        instruction: 'Check total key count',
        command: 'DBSIZE',
        explanation: 'KEYS scans all keys - O(n) operation!'
      }
    ]
  },
  {
    id: 'cache-stampede',
    title: 'Preventing Cache Stampede',
    duration: '20 min',
    difficulty: 'Advanced',
    icon: <AlertCircle className="w-5 h-5" />,
    description: 'Handle the thundering herd problem when popular keys expire',
    objectives: [
      'Understand cache stampede scenario',
      'Implement prevention strategies',
      'Use probabilistic early expiration'
    ],
    steps: [
      {
        instruction: 'Set a popular key with TTL',
        command: 'SET hot:item "expensive-data" EX 60',
        explanation: 'Simulates frequently accessed data with expiration'
      },
      {
        instruction: 'Simulate multiple clients accessing',
        explanation: 'When key expires, all clients query DB simultaneously'
      },
      {
        instruction: 'Implement lock pattern',
        command: 'SET hot:item:lock "updating" NX EX 5',
        explanation: 'NX flag ensures only one client updates'
      },
      {
        instruction: 'Check if lock acquired',
        command: 'GET hot:item:lock',
        explanation: 'Other clients wait or use stale data'
      },
      {
        instruction: 'Update data and release',
        command: 'SET hot:item "new-data" EX 60',
        explanation: 'Single DB query instead of stampede'
      },
      {
        instruction: 'Delete lock',
        command: 'DEL hot:item:lock',
        explanation: 'Allow future updates when needed'
      }
    ]
  }
];

interface WorkshopScenariosProps {
  isOpen: boolean;
  onClose: () => void;
}

export function WorkshopScenarios({ isOpen, onClose }: WorkshopScenariosProps) {
  const [selectedScenario, setSelectedScenario] = useState<Scenario | null>(null);
  const [currentStepIndex, setCurrentStepIndex] = useState(0);
  const [completedSteps, setCompletedSteps] = useState<Set<number>>(new Set());
  const [isRunning, setIsRunning] = useState(false);
  const { hitRatio } = useAppStore();

  const handleStartScenario = (scenario: Scenario) => {
    setSelectedScenario(scenario);
    setCurrentStepIndex(0);
    setCompletedSteps(new Set());
    setIsRunning(true);
  };

  const handleCompleteStep = () => {
    setCompletedSteps(prev => new Set(prev).add(currentStepIndex));
    if (currentStepIndex < (selectedScenario?.steps.length || 0) - 1) {
      setCurrentStepIndex(prev => prev + 1);
    } else {
      setIsRunning(false);
    }
  };

  const handleReset = () => {
    setCurrentStepIndex(0);
    setCompletedSteps(new Set());
    setIsRunning(true);
  };

  const getDifficultyColor = (difficulty: string) => {
    switch (difficulty) {
      case 'Beginner': return 'text-green-400';
      case 'Intermediate': return 'text-yellow-400';
      case 'Advanced': return 'text-red-400';
      default: return 'text-gray-400';
    }
  };

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/50 backdrop-blur-sm">
      <Card className="w-[90%] max-w-5xl h-[85vh] flex flex-col bg-gray-900 border-gray-700 text-gray-100">
        <CardHeader className="pb-3">
          <div className="flex items-center justify-between">
            <div>
              <CardTitle className="text-2xl">Workshop Scenarios</CardTitle>
              <CardDescription className="text-gray-400">
                Interactive Redis learning scenarios for your workshop
              </CardDescription>
            </div>
            <Button
              variant="ghost"
              onClick={onClose}
              className="hover:bg-gray-800"
            >
              ✕
            </Button>
          </div>
        </CardHeader>

        <CardContent className="flex-1 overflow-hidden flex gap-4 p-4">
          {!selectedScenario ? (
            // Scenario Selection Grid
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4 w-full overflow-y-auto">
              {SCENARIOS.map((scenario) => (
                <Card
                  key={scenario.id}
                  className="bg-gray-800 border-gray-700 hover:border-gray-600 cursor-pointer transition-colors"
                  onClick={() => handleStartScenario(scenario)}
                >
                  <CardHeader className="pb-3">
                    <div className="flex items-start justify-between">
                      <div className="flex items-center gap-2">
                        {scenario.icon}
                        <CardTitle className="text-lg">{scenario.title}</CardTitle>
                      </div>
                      <div className="flex gap-2">
                        <Badge variant="outline" className="text-xs">
                          {scenario.duration}
                        </Badge>
                        <Badge
                          variant="outline"
                          className={`text-xs ${getDifficultyColor(scenario.difficulty)}`}
                        >
                          {scenario.difficulty}
                        </Badge>
                      </div>
                    </div>
                  </CardHeader>
                  <CardContent>
                    <p className="text-sm text-gray-400 mb-3">{scenario.description}</p>
                    <div className="space-y-1">
                      <p className="text-xs font-semibold text-gray-500">Learning Objectives:</p>
                      <ul className="text-xs text-gray-400 list-disc list-inside">
                        {scenario.objectives.map((obj, idx) => (
                          <li key={idx}>{obj}</li>
                        ))}
                      </ul>
                    </div>
                  </CardContent>
                </Card>
              ))}
            </div>
          ) : (
            // Scenario Execution View
            <div className="flex w-full gap-4">
              {/* Steps Panel */}
              <div className="w-1/3 overflow-y-auto">
                <div className="mb-4">
                  <Button
                    variant="ghost"
                    size="sm"
                    onClick={() => setSelectedScenario(null)}
                    className="mb-2"
                  >
                    ← Back to Scenarios
                  </Button>
                  <h3 className="text-lg font-semibold">{selectedScenario.title}</h3>
                  <p className="text-sm text-gray-400">{selectedScenario.description}</p>
                </div>

                <Separator className="bg-gray-700 mb-4" />

                <div className="space-y-2">
                  {selectedScenario.steps.map((step, idx) => (
                    <div
                      key={idx}
                      className={`p-3 rounded-lg border ${
                        idx === currentStepIndex
                          ? 'bg-blue-900/30 border-blue-600'
                          : completedSteps.has(idx)
                          ? 'bg-green-900/20 border-green-800'
                          : 'bg-gray-800 border-gray-700'
                      }`}
                    >
                      <div className="flex items-center gap-2 mb-1">
                        {completedSteps.has(idx) ? (
                          <CheckCircle className="w-4 h-4 text-green-400" />
                        ) : idx === currentStepIndex ? (
                          <div className="w-4 h-4 border-2 border-blue-400 rounded-full" />
                        ) : (
                          <div className="w-4 h-4 border-2 border-gray-600 rounded-full" />
                        )}
                        <span className="text-sm font-medium">Step {idx + 1}</span>
                      </div>
                      <p className="text-xs text-gray-400 ml-6">
                        {step.instruction}
                      </p>
                    </div>
                  ))}
                </div>

                <div className="mt-4 flex gap-2">
                  <Button
                    size="sm"
                    onClick={handleReset}
                    variant="outline"
                  >
                    <RotateCcw className="w-4 h-4 mr-1" />
                    Reset
                  </Button>
                  {isRunning ? (
                    <Button
                      size="sm"
                      onClick={() => setIsRunning(false)}
                      variant="outline"
                    >
                      <PauseCircle className="w-4 h-4 mr-1" />
                      Pause
                    </Button>
                  ) : (
                    <Button
                      size="sm"
                      onClick={() => setIsRunning(true)}
                      variant="outline"
                    >
                      <PlayCircle className="w-4 h-4 mr-1" />
                      Resume
                    </Button>
                  )}
                </div>
              </div>

              {/* Current Step Detail */}
              <div className="flex-1 overflow-y-auto">
                {selectedScenario.steps[currentStepIndex] && (
                  <Card className="bg-gray-800 border-gray-700">
                    <CardHeader>
                      <CardTitle className="text-lg">
                        Step {currentStepIndex + 1} of {selectedScenario.steps.length}
                      </CardTitle>
                    </CardHeader>
                    <CardContent className="space-y-4">
                      <div>
                        <h4 className="font-semibold mb-2">Instruction:</h4>
                        <p className="text-gray-300">
                          {selectedScenario.steps[currentStepIndex].instruction}
                        </p>
                      </div>

                      {selectedScenario.steps[currentStepIndex].command && (
                        <div>
                          <h4 className="font-semibold mb-2">Command to Run:</h4>
                          <div className="bg-black p-3 rounded font-mono text-sm">
                            <span className="text-green-400">redis&gt;</span>{' '}
                            {selectedScenario.steps[currentStepIndex].command}
                          </div>
                        </div>
                      )}

                      {selectedScenario.steps[currentStepIndex].expectedResult && (
                        <div>
                          <h4 className="font-semibold mb-2">Expected Result:</h4>
                          <div className="bg-gray-900 p-3 rounded font-mono text-sm text-yellow-300">
                            {selectedScenario.steps[currentStepIndex].expectedResult}
                          </div>
                        </div>
                      )}

                      <div>
                        <h4 className="font-semibold mb-2">Explanation:</h4>
                        <p className="text-gray-400">
                          {selectedScenario.steps[currentStepIndex].explanation}
                        </p>
                      </div>

                      {/* Live Stats */}
                      <div className="bg-gray-900 p-4 rounded">
                        <h4 className="font-semibold mb-2">Live Statistics:</h4>
                        <div className="grid grid-cols-3 gap-4 text-sm">
                          <div>
                            <p className="text-gray-500">Hit Ratio</p>
                            <p className="text-xl font-bold">
                              {(hitRatio.ratio * 100).toFixed(1)}%
                            </p>
                          </div>
                          <div>
                            <p className="text-gray-500">Total Hits</p>
                            <p className="text-xl font-bold text-green-400">
                              {hitRatio.hits}
                            </p>
                          </div>
                          <div>
                            <p className="text-gray-500">Total Misses</p>
                            <p className="text-xl font-bold text-red-400">
                              {hitRatio.misses}
                            </p>
                          </div>
                        </div>
                      </div>

                      <Button
                        onClick={handleCompleteStep}
                        className="w-full"
                        disabled={!isRunning}
                      >
                        {currentStepIndex < selectedScenario.steps.length - 1
                          ? 'Complete & Next Step'
                          : 'Complete Scenario'}
                      </Button>
                    </CardContent>
                  </Card>
                )}

                {!isRunning && completedSteps.size === selectedScenario.steps.length && (
                  <Card className="bg-green-900/30 border-green-700 mt-4">
                    <CardContent className="pt-6">
                      <div className="flex items-center gap-3 mb-4">
                        <Trophy className="w-8 h-8 text-yellow-400" />
                        <div>
                          <h3 className="text-xl font-semibold">Scenario Complete!</h3>
                          <p className="text-gray-400">
                            You\'ve successfully completed "{selectedScenario.title}"
                          </p>
                        </div>
                      </div>
                      <div className="flex gap-2">
                        <Button
                          onClick={() => setSelectedScenario(null)}
                          variant="outline"
                        >
                          Try Another Scenario
                        </Button>
                        <Button onClick={handleReset}>
                          Replay This Scenario
                        </Button>
                      </div>
                    </CardContent>
                  </Card>
                )}
              </div>
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  );
}