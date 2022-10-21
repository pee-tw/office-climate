import * as functions from "firebase-functions";
import * as admin from "firebase-admin";
import axios from "axios";
import dayjs from "dayjs";

const frontendUrl = "https://office-sensors-27e21.web.app";
const apiKey = "q3yTIkBnN5qNfbzprZq2Ce4jzUH40GA4";

export const WeatherRoute = functions
  .runWith({ timeoutSeconds: 10, memory: "128MB" })
  .region("asia-southeast1")
  .https.onRequest(async (req, response) => {
    response.set("Access-Control-Allow-Origin", frontendUrl);
    response.set("Access-Control-Allow-Methods", "GET, POST");

    if (req.method === "OPTIONS") {
      response.status(204).send("");
      return;
    }

    const firestore = admin.firestore();

    const weatherDocs = await firestore
      .collection("weather")
      .where("retrieved", ">", dayjs().subtract(30, "minute").toDate())
      .limit(1)
      .get();

    if (weatherDocs.size > 0) {
      response.send(weatherDocs.docs[0].data());
      return;
    }

    const { data } = await axios.get(
      `https://dataservice.accuweather.com/currentconditions/v1/318849?apikey=${apiKey}`
    );

    firestore.collection("weather").add({ ...data[0], retrieved: new Date() });
    
    response.send(data[0]);
  });
