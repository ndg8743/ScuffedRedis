'use client';

import React, { useState } from 'react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Separator } from '@/components/ui/separator';
import {
  X,
  ChevronRight,
  ChevronLeft,
  BookOpen,
  Code,
  Database,
  Network,
  Timer,
  TrendingUp,
  Layers,
  GitBranch,
  Hash
} from 'lucide-react';

interface PopupContent {
  title: string;
  icon: React.ReactNode;
  description: string;
  content: React.ReactNode;
}

const POPUP_CONTENTS: PopupContent[] = [
  {
    title: 'How Redis Caching Works',
    icon: <Database className="w-5 h-5" />,
    description: 'Understanding the cache-aside pattern',
    content: (
      <div className="space-y-4">
        <div className="bg-gray-800 p-4 rounded-lg">
          <h4 className="font-semibold mb-2">Cache-Aside Pattern</h4>
          <ol className="list-decimal list-inside space-y-2 text-sm">
            <li>Application requests data</li>
            <li>Check cache first (Redis) - Fast! ~1ms</li>
            <li>If cache hit: Return data immediately</li>
            <li>If cache miss: Query database - Slow! ~100-1000ms</li>
            <li>Store result in cache for future requests</li>
            <li>Return data to application</li>
          </ol>
        </div>

        <div className="grid grid-cols-2 gap-4">
          <div className="bg-green-900/30 p-3 rounded">
            <h5 className="font-medium text-green-400 mb-1">Cache Hit</h5>
            <p className="text-sm">Data found in Redis</p>
            <p className="text-xs text-gray-400 mt-1">Latency: &lt;10ms</p>
          </div>
          <div className="bg-red-900/30 p-3 rounded">
            <h5 className="font-medium text-red-400 mb-1">Cache Miss</h5>
            <p className="text-sm">Query database</p>
            <p className="text-xs text-gray-400 mt-1">Latency: 600-1200ms</p>
          </div>
        </div>

        <div className="bg-blue-900/20 p-3 rounded">
          <p className="text-sm">
            <strong>Pro Tip:</strong> A high hit ratio (80%+) means your cache is effective.
            Watch the visualization to see how warming the cache improves performance!
          </p>
        </div>
      </div>
    )
  },
  {
    title: 'Hash Table Implementation',
    icon: <Hash className="w-5 h-5" />,
    description: 'How Redis stores your data internally',
    content: (
      <div className="space-y-4">
        <div className="bg-gray-800 p-4 rounded-lg">
          <h4 className="font-semibold mb-2">Hash Table Structure</h4>
          <div className="space-y-2 text-sm">
            <p>Redis uses a hash table for O(1) operations:</p>
            <ul className="list-disc list-inside space-y-1 ml-2">
              <li>Buckets array with linked lists for collisions</li>
              <li>MurmurHash3 for even distribution</li>
              <li>Dynamic resizing at 75% load factor</li>
              <li>Thread-safe with reader-writer locks</li>
            </ul>
          </div>
        </div>

        <div className="bg-gray-800 p-3 rounded-lg font-mono text-xs">
          <div className="text-green-400 mb-1">// Simplified hash table operation</div>
          <div>uint32_t hash = murmur3(key);</div>
          <div>size_t index = hash % bucket_count;</div>
          <div>bucket[index].insert(key, value);</div>
        </div>

        <div className="grid grid-cols-3 gap-2 text-center">
          <div className="bg-gray-800 p-2 rounded">
            <p className="text-xs text-gray-400">GET/SET</p>
            <p className="font-semibold">O(1)</p>
          </div>
          <div className="bg-gray-800 p-2 rounded">
            <p className="text-xs text-gray-400">DEL</p>
            <p className="font-semibold">O(1)</p>
          </div>
          <div className="bg-gray-800 p-2 rounded">
            <p className="text-xs text-gray-400">KEYS</p>
            <p className="font-semibold">O(n)</p>
          </div>
        </div>

        <div className="bg-yellow-900/20 p-3 rounded">
          <p className="text-sm">
            <strong>Note:</strong> The full server uses a custom hash table,
            while the simple server uses std::map (O(log n)).
          </p>
        </div>
      </div>
    )
  },
  {
    title: 'Binary Protocol',
    icon: <Network className="w-5 h-5" />,
    description: 'Understanding the wire format',
    content: (
      <div className="space-y-4">
        <div className="bg-gray-800 p-4 rounded-lg">
          <h4 className="font-semibold mb-2">Message Format</h4>
          <div className="font-mono text-xs space-y-1">
            <div>[Type: 1 byte][Length: 4 bytes LE][Data: N bytes]</div>
          </div>
        </div>

        <div className="space-y-2">
          <h5 className="font-medium">Message Types:</h5>
          <div className="grid grid-cols-2 gap-2 text-sm">
            <div className="bg-gray-800 p-2 rounded">
              <span className="font-mono text-xs">0x01</span> - Simple String
            </div>
            <div className="bg-gray-800 p-2 rounded">
              <span className="font-mono text-xs">0x02</span> - Error
            </div>
            <div className="bg-gray-800 p-2 rounded">
              <span className="font-mono text-xs">0x03</span> - Integer
            </div>
            <div className="bg-gray-800 p-2 rounded">
              <span className="font-mono text-xs">0x04</span> - Bulk String
            </div>
            <div className="bg-gray-800 p-2 rounded">
              <span className="font-mono text-xs">0x05</span> - Array
            </div>
            <div className="bg-gray-800 p-2 rounded">
              <span className="font-mono text-xs">0x06</span> - Null
            </div>
          </div>
        </div>

        <div className="bg-gray-800 p-3 rounded-lg">
          <h5 className="font-medium mb-2">Example: SET key value</h5>
          <div className="font-mono text-xs space-y-1">
            <div className="text-green-400">// Array of 3 elements</div>
            <div>[0x05][0x03 0x00 0x00 0x00]</div>
            <div className="ml-4 text-gray-400">// "SET"</div>
            <div className="ml-4">[0x04][0x03 0x00 0x00 0x00]SET</div>
            <div className="ml-4 text-gray-400">// "key"</div>
            <div className="ml-4">[0x04][0x03 0x00 0x00 0x00]key</div>
            <div className="ml-4 text-gray-400">// "value"</div>
            <div className="ml-4">[0x04][0x05 0x00 0x00 0x00]value</div>
          </div>
        </div>
      </div>
    )
  },
  {
    title: 'AVL Tree for Sorted Sets',
    icon: <GitBranch className="w-5 h-5" />,
    description: 'Self-balancing binary search tree',
    content: (
      <div className="space-y-4">
        <div className="bg-gray-800 p-4 rounded-lg">
          <h4 className="font-semibold mb-2">AVL Tree Properties</h4>
          <ul className="list-disc list-inside space-y-1 text-sm">
            <li>Height-balanced: |left_height - right_height| ≤ 1</li>
            <li>Automatic rebalancing on insert/delete</li>
            <li>Guarantees O(log n) operations</li>
            <li>Used for ZRANGE, ZRANK operations</li>
          </ul>
        </div>

        <div className="bg-gray-800 p-3 rounded-lg">
          <h5 className="font-medium mb-2">Sorted Set Commands</h5>
          <div className="space-y-2 text-sm font-mono">
            <div>ZADD leaderboard 100 alice</div>
            <div>ZADD leaderboard 95 bob</div>
            <div>ZRANGE leaderboard 0 -1</div>
            <div className="text-green-400">→ ["bob", "alice"]</div>
          </div>
        </div>

        <div className="grid grid-cols-3 gap-2 text-center">
          <div className="bg-gray-800 p-2 rounded">
            <p className="text-xs text-gray-400">ZADD</p>
            <p className="font-semibold">O(log n)</p>
          </div>
          <div className="bg-gray-800 p-2 rounded">
            <p className="text-xs text-gray-400">ZRANGE</p>
            <p className="font-semibold">O(log n + m)</p>
          </div>
          <div className="bg-gray-800 p-2 rounded">
            <p className="text-xs text-gray-400">ZRANK</p>
            <p className="font-semibold">O(log n)</p>
          </div>
        </div>

        <div className="bg-blue-900/20 p-3 rounded">
          <p className="text-sm">
            <strong>Use Case:</strong> Perfect for leaderboards, priority queues,
            and time-series data with scores.
          </p>
        </div>
      </div>
    )
  },
  {
    title: 'TTL & Expiration',
    icon: <Timer className="w-5 h-5" />,
    description: 'Automatic key expiration management',
    content: (
      <div className="space-y-4">
        <div className="bg-gray-800 p-4 rounded-lg">
          <h4 className="font-semibold mb-2">TTL Manager</h4>
          <ul className="list-disc list-inside space-y-1 text-sm">
            <li>Min-heap for efficient expiration tracking</li>
            <li>Millisecond precision timestamps</li>
            <li>Background cleanup thread</li>
            <li>Lazy deletion on access</li>
          </ul>
        </div>

        <div className="bg-gray-800 p-3 rounded-lg">
          <h5 className="font-medium mb-2">Setting TTL</h5>
          <div className="font-mono text-sm space-y-1">
            <div className="text-green-400"># Expire in 60 seconds</div>
            <div>SET session:123 data EX 60</div>
            <div className="mt-2 text-green-400"># Check remaining TTL</div>
            <div>TTL session:123</div>
            <div className="text-yellow-300">→ 45 (seconds left)</div>
          </div>
        </div>

        <div className="bg-yellow-900/20 p-3 rounded">
          <p className="text-sm">
            <strong>Workshop Demo:</strong> Set keys with different TTLs
            and watch them expire in real-time!
          </p>
        </div>
      </div>
    )
  },
  {
    title: 'Performance Metrics',
    icon: <TrendingUp className="w-5 h-5" />,
    description: 'Understanding cache performance',
    content: (
      <div className="space-y-4">
        <div className="bg-gray-800 p-4 rounded-lg">
          <h4 className="font-semibold mb-2">Key Metrics</h4>
          <div className="space-y-3">
            <div>
              <h5 className="font-medium text-green-400">Hit Ratio</h5>
              <p className="text-sm">Percentage of requests served from cache</p>
              <p className="text-xs text-gray-400">Target: &gt;80% for most workloads</p>
            </div>
            <div>
              <h5 className="font-medium text-blue-400">Latency</h5>
              <p className="text-sm">Response time for operations</p>
              <p className="text-xs text-gray-400">Redis: &lt;1ms, Database: 100-1000ms</p>
            </div>
            <div>
              <h5 className="font-medium text-purple-400">QPS</h5>
              <p className="text-sm">Queries per second handled</p>
              <p className="text-xs text-gray-400">Redis: 100k+, Database: 1-10k</p>
            </div>
          </div>
        </div>

        <div className="bg-blue-900/20 p-3 rounded">
          <h5 className="font-medium mb-1">Zipf Distribution</h5>
          <p className="text-sm">
            Our traffic generator uses Zipf distribution to simulate real-world
            access patterns where 20% of keys get 80% of traffic.
          </p>
        </div>
      </div>
    )
  }
];

interface EducationalPopupsProps {
  isOpen: boolean;
  onClose: () => void;
}

export function EducationalPopups({ isOpen, onClose }: EducationalPopupsProps) {
  const [currentIndex, setCurrentIndex] = useState(0);

  const handleNext = () => {
    setCurrentIndex((prev) => (prev + 1) % POPUP_CONTENTS.length);
  };

  const handlePrevious = () => {
    setCurrentIndex((prev) => (prev - 1 + POPUP_CONTENTS.length) % POPUP_CONTENTS.length);
  };

  if (!isOpen) return null;

  const current = POPUP_CONTENTS[currentIndex];

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/50 backdrop-blur-sm">
      <Card className="w-[90%] max-w-2xl max-h-[80vh] overflow-hidden bg-gray-900 border-gray-700 text-gray-100">
        <CardHeader className="pb-3">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              {current.icon}
              <CardTitle className="text-xl">{current.title}</CardTitle>
            </div>
            <Button
              variant="ghost"
              size="icon"
              onClick={onClose}
              className="hover:bg-gray-800"
            >
              <X className="w-4 h-4" />
            </Button>
          </div>
          <CardDescription className="text-gray-400">
            {current.description}
          </CardDescription>
        </CardHeader>

        <CardContent className="overflow-y-auto max-h-[50vh]">
          {current.content}
        </CardContent>

        <Separator className="bg-gray-700" />

        <div className="p-4 flex items-center justify-between">
          <div className="flex items-center gap-2">
            <BookOpen className="w-4 h-4 text-gray-400" />
            <span className="text-sm text-gray-400">
              {currentIndex + 1} of {POPUP_CONTENTS.length}
            </span>
          </div>

          <div className="flex gap-2">
            <Button
              variant="outline"
              size="sm"
              onClick={handlePrevious}
              className="gap-1"
            >
              <ChevronLeft className="w-4 h-4" />
              Previous
            </Button>
            <Button
              variant="outline"
              size="sm"
              onClick={handleNext}
              className="gap-1"
            >
              Next
              <ChevronRight className="w-4 h-4" />
            </Button>
          </div>
        </div>
      </Card>
    </div>
  );
}