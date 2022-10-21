import * as functions from "firebase-functions";

import * as admin from "firebase-admin";
import { API_KEY } from "../api-key";

export const OfficeMovementRoute = functions
  .runWith({ timeoutSeconds: 10, memory: "128MB" })
  .region("asia-southeast1")
  .https.onRequest((request, response) => {
    if (request.headers["x-api-key"] != API_KEY) {
      response.sendStatus(401);
    } else {
      const payload = {
        dateTime: new Date(),
      };
      admin.firestore().collection("office-movement").add(payload);
      // Send back a message that we've succesfully written the message
      response.sendStatus(201);
    }
  });
