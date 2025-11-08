'use client';

import { useRef, useMemo, useEffect, useState } from 'react';
import { useFrame } from '@react-three/fiber';
import { InstancedMesh, Matrix4, Vector3, Color } from 'three';
import { CONFIG } from '@/lib/config';
import { useAppStore, CommandType } from '@/lib/state';
import { getCellIndex } from '@/lib/utils';

interface CellState {
  colorTimer: number;
  pulseTimer: number;
  targetColor: [number, number, number];
  currentColor: [number, number, number];
  scale: number;
  targetScale: number;
  pulsePattern: string;
  keyName: string;
  command: CommandType;
}

export function Heatmap() {
  const meshRef = useRef<InstancedMesh>(null);
  const { events, operationsPerSecond } = useAppStore();
  const [hoveredCell, setHoveredCell] = useState<{ index: number; info: string } | null>(null);

  // Initialize cell states
  const cellStates = useMemo(() => {
    const states: CellState[] = [];
    for (let i = 0; i < CONFIG.TOTAL_CELLS; i++) {
      states.push({
        colorTimer: 0,
        pulseTimer: 0,
        targetColor: CONFIG.COLORS.NEUTRAL,
        currentColor: CONFIG.COLORS.NEUTRAL,
        scale: 1,
        targetScale: 1,
        pulsePattern: 'smooth',
        keyName: '',
        command: 'OTHER'
      });
    }
    return states;
  }, []);

  // Get color based on command type
  const getCommandColor = (command: CommandType): [number, number, number] => {
    return CONFIG.COMMAND_COLORS[command] || CONFIG.COMMAND_COLORS.OTHER;
  };

  // Process new events with command type awareness
  useEffect(() => {
    if (events.length === 0) return;

    const latestEvent = events[events.length - 1];
    const cellIndex = getCellIndex(latestEvent.id, CONFIG.TOTAL_CELLS);
    const cellState = cellStates[cellIndex];

    if (cellState) {
      cellState.command = latestEvent.command || 'OTHER';
      cellState.keyName = latestEvent.key || `Key ${latestEvent.id}`;
      cellState.pulsePattern = CONFIG.ANIMATION_PATTERNS[cellState.command];

      // Use command-specific color if available, otherwise fall back to hit/miss
      cellState.targetColor = getCommandColor(cellState.command);

      // Enhance scale based on animation pattern
      switch (cellState.pulsePattern) {
        case 'sharp':
          cellState.targetScale = 1.3; // DEL gets larger spike
          break;
        case 'bounce':
          cellState.targetScale = 1.15; // SET gets moderate bounce
          break;
        case 'wave':
          cellState.targetScale = 1.2; // INCR gets wave motion
          break;
        default:
          cellState.targetScale = 1.15;
      }

      // Reset timers
      cellState.colorTimer = CONFIG.COLOR_FADE_DURATION;
      cellState.pulseTimer = CONFIG.PULSE_DURATION;
    }
  }, [events, cellStates]);

  // Initialize instance matrices on first render
  useEffect(() => {
    if (!meshRef.current) return;

    const matrix = new Matrix4();
    const position = new Vector3();

    for (let i = 0; i < CONFIG.TOTAL_CELLS; i++) {
      const row = Math.floor(i / CONFIG.WALL_COLS);
      const col = i % CONFIG.WALL_COLS;

      position.set(
        (col - CONFIG.WALL_COLS / 2) * 1.1,
        (row - CONFIG.WALL_ROWS / 2) * 1.1,
        0
      );

      matrix.setPosition(position);
      meshRef.current.setMatrixAt(i, matrix);

      // Set initial color
      const color = new Color(...CONFIG.COLORS.NEUTRAL);
      meshRef.current.setColorAt(i, color);
    }

    meshRef.current.instanceMatrix.needsUpdate = true;
    if (meshRef.current.instanceColor) {
      meshRef.current.instanceColor.needsUpdate = true;
    }
  }, []);

  // Animation loop with pattern-specific pulse behaviors
  useFrame((state, delta) => {
    if (!meshRef.current) return;

    const deltaMs = delta * 1000;

    cellStates.forEach((cellState, index) => {
      // Update color animation
      if (cellState.colorTimer > 0) {
        cellState.colorTimer -= deltaMs;

        // Smooth lerp to target color
        const t = 0.1;
        cellState.currentColor = [
          cellState.currentColor[0] + (cellState.targetColor[0] - cellState.currentColor[0]) * t,
          cellState.currentColor[1] + (cellState.targetColor[1] - cellState.currentColor[1]) * t,
          cellState.currentColor[2] + (cellState.targetColor[2] - cellState.currentColor[2]) * t,
        ] as [number, number, number];

        // Fade back to neutral when timer expires
        if (cellState.colorTimer <= 0) {
          cellState.targetColor = CONFIG.COLORS.NEUTRAL;
        }
      } else {
        // Fade to neutral
        const t = 0.05;
        cellState.currentColor = [
          cellState.currentColor[0] + (CONFIG.COLORS.NEUTRAL[0] - cellState.currentColor[0]) * t,
          cellState.currentColor[1] + (CONFIG.COLORS.NEUTRAL[1] - cellState.currentColor[1]) * t,
          cellState.currentColor[2] + (CONFIG.COLORS.NEUTRAL[2] - cellState.currentColor[2]) * t,
        ] as [number, number, number];
      }

      // Update pulse animation with pattern variations
      if (cellState.pulseTimer > 0) {
        cellState.pulseTimer -= deltaMs;

        const pulseProgress = 1 - (cellState.pulseTimer / CONFIG.PULSE_DURATION);

        // Different pulse patterns for different operations
        let pulseScale = 1;
        switch (cellState.pulsePattern) {
          case 'smooth':
            // Smooth sine wave
            pulseScale = 1 + Math.sin(pulseProgress * Math.PI) * 0.2;
            break;
          case 'bounce':
            // Bouncy dual pulse
            pulseScale = 1 + Math.sin(pulseProgress * Math.PI * 2) * 0.15;
            break;
          case 'sharp':
            // Quick sharp spike
            pulseScale = 1 + Math.max(0, 1 - pulseProgress) * 0.3;
            break;
          case 'wave':
            // Wave with decay
            pulseScale = 1 + Math.sin(pulseProgress * Math.PI * 3) * (0.2 * (1 - pulseProgress));
            break;
          case 'fade':
            // Fade without size change
            pulseScale = 1 + Math.cos(pulseProgress * Math.PI) * 0.1;
            break;
          default:
            pulseScale = 1 + Math.sin(pulseProgress * Math.PI) * 0.2;
        }

        cellState.scale = pulseScale;
      } else {
        // Return to normal scale
        cellState.scale += (1 - cellState.scale) * 0.1;
      }

      // Update instance color
      const color = new Color(
        cellState.currentColor[0],
        cellState.currentColor[1],
        cellState.currentColor[2]
      );
      meshRef.current.setColorAt(index, color);
    });

    meshRef.current.instanceColor!.needsUpdate = true;
  });

  return (
    <>
      <instancedMesh
        ref={meshRef}
        args={[undefined, undefined, CONFIG.TOTAL_CELLS]}
        position={[0, 0, 0]}
      >
        <boxGeometry args={[0.8, 0.8, 0.8]} />
        <meshStandardMaterial
          color="white"
          emissive={[0.1, 0.1, 0.1]}
          emissiveIntensity={0.2}
          toneMapped={false}
        />
      </instancedMesh>

      {/* Real-time operations counter overlay - positioned in top-left */}
      <group position={[-8, 8, 5]}>
        <mesh position={[0, 0, 0]}>
          <planeGeometry args={[3, 1.5]} />
          <meshBasicMaterial color="#000000" transparent opacity={0.7} />
        </mesh>
      </group>
    </>
  );
}
