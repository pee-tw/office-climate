import { mount } from "@vue/test-utils";
import OfficeSensor from "./OfficeSensor.vue";

describe("Office Sensors", () => {
  it("should mount component", async () => {
    expect(OfficeSensor).toBeTruthy();

    const wrapper = mount(OfficeSensor);

    expect(wrapper.text()).toContain("Temp: ");
  });
});
