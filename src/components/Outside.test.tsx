import "@testing-library/jest-dom";
import { render, screen } from "@testing-library/react";
import axios from "axios";
import { describe, expect, it } from "vitest";
import Outside from "./Outside";

describe("Outside", () => {
  beforeEach(() => {
    vi.restoreAllMocks();
  });
  it("the title is visible", async () => {
    const mockedResponse = {
      data: {
        WeatherIcon: 1,
        WeatherText: "Sunny",
        Temperature: {
          Metric: {
            Value: 30,
          },
        },
      },
    };

    vi.spyOn(axios, "get").mockResolvedValue(mockedResponse);

    render(<Outside />);

    expect(await screen.findByText("The Weather Outside")).toBeInTheDocument();
  });
});
