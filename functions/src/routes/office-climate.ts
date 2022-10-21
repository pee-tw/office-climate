import * as functions from "firebase-functions";

import * as admin from "firebase-admin";
import { API_KEY } from "../api-key";

export const OfficeClimateRoute = functions
  .runWith({ timeoutSeconds: 10, memory: "128MB" })
  .region("asia-southeast1")
  .https.onRequest((request, response) => {
    const firestore = admin.firestore();
    const { temperature, humidity } = request.body;
    
    if (request.headers["x-api-key"] != API_KEY) {
      response.sendStatus(401);
    } else if (!temperature || !humidity) {
      response.sendStatus(400);
    } else {
      const payload = {
        dateTime: new Date(),
        temperature,
        humidity,
      };

      firestore.collection("office-climate").add(payload);

      firestore.collection("office-climate-latest").doc("latest").set(payload);

      // Send back a message that we've succesfully written the message
      response.sendStatus(201);
    }
  });
