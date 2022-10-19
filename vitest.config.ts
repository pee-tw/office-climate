/// <reference types="vitest" />
/// <reference types="vite/client" />

import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import Vue from "@vitejs/plugin-vue";

export default defineConfig({
  plugins: [Vue(), react()],
  test: {
    globals: true,
    environment: "jsdom",
  },
});
