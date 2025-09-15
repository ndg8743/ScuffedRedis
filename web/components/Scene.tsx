'use client';

import { Canvas } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import { Heatmap } from './Heatmap';

export function Scene() {
  return (
    <Canvas
      camera={{ position: [15, 15, 15], fov: 50 }}
      style={{ background: 'linear-gradient(to bottom, #0a0a0a, #1a1a1a)' }}
    >
      {/* Lighting */}
      <ambientLight intensity={0.3} />
      <directionalLight
        position={[10, 10, 5]}
        intensity={0.8}
        castShadow
        shadow-mapSize-width={2048}
        shadow-mapSize-height={2048}
      />
      
      {/* Simple environment lighting */}
      <fog attach="fog" args={['#000000', 10, 100]} />
      
      {/* 3D Heatmap */}
      <Heatmap />
      
      {/* Camera Controls */}
      <OrbitControls
        enablePan={true}
        enableZoom={true}
        enableRotate={true}
        dampingFactor={0.05}
        autoRotate={true}
        autoRotateSpeed={0.5}
        minDistance={5}
        maxDistance={50}
      />
    </Canvas>
  );
}
