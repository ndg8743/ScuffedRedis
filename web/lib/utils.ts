import { type ClassValue, clsx } from "clsx"
import { twMerge } from "tailwind-merge"

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

// FNV-1a hash function for deterministic cell mapping
export function fnv1a(str: string): number {
  let hash = 2166136261;
  for (let i = 0; i < str.length; i++) {
    hash ^= str.charCodeAt(i);
    hash *= 16777619;
  }
  return hash >>> 0; // Convert to unsigned 32-bit integer
}

// Map item ID to cell index using hash
export function getCellIndex(id: number, totalCells: number): number {
  return fnv1a(id.toString()) % totalCells;
}





