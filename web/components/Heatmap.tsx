'use client';

import { useRef, useMemo, useEffect } from 'react';
import { useFrame } from '@react-three/fiber';
import { InstancedMesh, Matrix4, Vector3, Color } from 'three';
import { CONFIG } from '@/lib/config';
import { useAppStore } from '@/lib/state';
import { getCellIndex } from '@/lib/utils';

interface CellState {
  colorTimer: number;
  pulseTimer: number;
  targetColor: [number, number, number];
  currentColor: [number, number, number];
  scale: number;
  targetScale: number;
}

export function Heatmap() {
  const meshRef = useRef<InstancedMesh>(null);
  const { events } = useAppStore();
  
  // Initialize cell states
  const cellStates = useMemo(() => {
    const states: CellState[] = [];
    for (let i = 0; i < CONFIG.TOTAL_CELLS; i++) {
      states.push({
        colorTimer: 0,
        pulseTimer: 0,
        targetColor: [...CONFIG.COLORS.NEUTRAL] as [number, number, number],
        currentColor: [...CONFIG.COLORS.NEUTRAL] as [number, number, number],
        scale: 1,
        targetScale: 1,
      });
    }
    return states;
  }, []);

  // Process new events
  useEffect(() => {
    if (events.length === 0) return;
    
    const latestEvent = events[events.length - 1];
    const cellIndex = getCellIndex(latestEvent.id, CONFIG.TOTAL_CELLS);
    const cellState = cellStates[cellIndex];
    
    if (cellState) {
      // Set target color based on hit/miss
      cellState.targetColor = [...(latestEvent.hit ? CONFIG.COLORS.HIT : CONFIG.COLORS.MISS)] as [number, number, number];
      cellState.targetScale = 1.2;
      
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

  // Animation loop
  useFrame((state, delta) => {
    if (!meshRef.current) return;
    const mesh = meshRef.current;

    const deltaMs = delta * 1000;

    cellStates.forEach((cellState: CellState, index: number) => {
      // Update color animation
      if (cellState.colorTimer > 0) {
        cellState.colorTimer -= deltaMs;
        
        // Lerp to target color
        const t = 1 - (cellState.colorTimer / CONFIG.COLOR_FADE_DURATION);
        cellState.currentColor = [
          cellState.currentColor[0] + (cellState.targetColor[0] - cellState.currentColor[0]) * t * 0.1,
          cellState.currentColor[1] + (cellState.targetColor[1] - cellState.currentColor[1]) * t * 0.1,
          cellState.currentColor[2] + (cellState.targetColor[2] - cellState.currentColor[2]) * t * 0.1,
        ] as [number, number, number];
        
        // Fade back to neutral when timer expires
        if (cellState.colorTimer <= 0) {
          cellState.targetColor = [...CONFIG.COLORS.NEUTRAL] as [number, number, number];
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
      
      // Update pulse animation
      if (cellState.pulseTimer > 0) {
        cellState.pulseTimer -= deltaMs;
        
        const pulseProgress = 1 - (cellState.pulseTimer / CONFIG.PULSE_DURATION);
        const pulseScale = 1 + Math.sin(pulseProgress * Math.PI) * 0.2;
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
      mesh.setColorAt(index, color);

      // Update scale (this would require updating the instance matrix)
      // For simplicity, we'll just use color changes
    });

    mesh.instanceColor!.needsUpdate = true;
  });

  return (
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
  );
}
