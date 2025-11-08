export const SERVER_URL = process.env.NEXT_PUBLIC_SERVER_URL || 'http://localhost:4000';

export const CONFIG = {
  // 3D Scene configuration
  WALL_ROWS: 20,
  WALL_COLS: 20,
  TOTAL_CELLS: 20 * 20, // 400 cells

  // Animation settings
  PULSE_DURATION: 900, // ms
  COLOR_FADE_DURATION: 2000, // ms

  // API polling
  HIT_RATIO_POLL_INTERVAL: 2000, // ms

  // Colors (RGB values for Three.js)
  COLORS: {
    HIT: [0, 1, 0],      // Green
    MISS: [1, 0, 0],     // Red
    IDLE: [0.2, 0.2, 0.2], // Dark gray
    NEUTRAL: [0.3, 0.3, 0.3] // Medium gray
  },

  // Command colors - specific operation type colors
  COMMAND_COLORS: {
    GET: [0, 1, 0],      // Green
    SET: [0, 0, 1],      // Blue
    DEL: [1, 0, 0],      // Red
    INCR: [1, 1, 0],     // Yellow
    EXPIRE: [1, 0.5, 0], // Orange
    OTHER: [0.5, 0.5, 1] // Light blue
  },

  // Animation patterns for different commands
  ANIMATION_PATTERNS: {
    GET: 'smooth',     // Smooth pulse
    SET: 'bounce',     // Bouncy pulse
    DEL: 'sharp',      // Sharp spike
    INCR: 'wave',      // Wave pulse
    EXPIRE: 'fade',    // Fade pulse
    OTHER: 'smooth'    // Smooth pulse
  }
} as const;
