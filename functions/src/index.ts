import * as admin from "firebase-admin";
import { OfficeClimateRoute } from "./routes/office-climate";
import { OfficeMovementRoute } from "./routes/office-movement";
import { WeatherRoute } from "./routes/weather"

admin.initializeApp();

export const officeClimate = OfficeClimateRoute;
export const officeMovement = OfficeMovementRoute;
export const weather = WeatherRoute;