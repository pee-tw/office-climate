<script setup lang="ts">
// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { doc, getFirestore, onSnapshot } from "firebase/firestore";
import { computed, reactive } from "vue";

import dayjs from "dayjs";
import localizedFormat from "dayjs/plugin/localizedFormat";
import { SensorTemp } from "../store/sensor-temp";

dayjs.extend(localizedFormat);

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyByi2BP_2T6fhdY7ujRAp1AQyT2rh92EDk",
  authDomain: "office-sensors-27e21.firebaseapp.com",
  projectId: "office-sensors-27e21",
  storageBucket: "office-sensors-27e21.appspot.com",
  messagingSenderId: "230838264020",
  appId: "1:230838264020:web:6f2cabfc8d977bccdfdb41",
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

const db = getFirestore(app);

const state = reactive({
  temp: 0,
  humidity: 0,
  lastUpdate: dayjs().format("LTS"),
});

const isCold = computed(() => {
  return state.temp < 22;
});

const isHot = computed(() => {
  return state.temp > 26;
});

const isComfy = computed(() => {
  return !isHot.value && !isCold.value;
});

onSnapshot(doc(db, "office-climate-latest", "latest"), (doc) => {
  const { humidity, temperature, dateTime } = doc.data();
  state.temp = temperature;
  state.humidity = humidity;
  state.lastUpdate = dayjs(dateTime.seconds * 1000).format("llll");

  SensorTemp.set(temperature);
});

const iconsConfig = {
  isCold: {
    url: "https://api.iconify.design/noto/cold-face.svg",
    alt: "Freezing",
  },
  isComfy: {
    url: "https://api.iconify.design/healthicons/happy-outline.svg",
    alt: "Comfy",
  },
  isHot: {
    url: "https://api.iconify.design/fluent-emoji/hot-face.svg",
    alt: "Hot",
  },
};

const currentIcon = computed(() => {
  if (isCold.value) return iconsConfig.isCold;
  if (isComfy.value) return iconsConfig.isComfy;
  if (isHot.value) return iconsConfig.isHot;
});
</script>

<template>
  <div class="pt-10 flex flex-col items-center">
    <img
      :src="currentIcon.url"
      width="128"
      height="128"
      :alt="currentIcon.alt"
    />
    <h1>Temp: {{ state.temp }}</h1>
    <h1>Humidity: {{ state.humidity }}</h1>
    <p>As of: {{ state.lastUpdate }}</p>
  </div>
</template>
