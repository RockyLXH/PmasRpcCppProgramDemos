#include "SingleAxisMotionGui.h"
#include <iostream>


int CallbackFunc(char* recv_buf, short buf_size, void* lpsock)
{
	switch (recv_buf[1])
	{
	case ASYNC_REPLY_EVT:
		printf("\n ASYNC event Reply ");
		break;
	case EMCY_EVT:
		printf("\n Emergency Event received ");
		break;
	case MOTIONENDED_EVT:
		printf("\n Motion Ended Event received ");
		break;
	case HBEAT_EVT:
		printf("\n H Beat Fail Event received ");
		break;
	case PDORCV_EVT:
		printf("\n PDO Received Event received - Updating Inputs ");
		break;
	case DRVERROR_EVT:
		printf("\n Drive Error Received Event received ");
		break;
	case HOME_ENDED_EVT:
		printf("\n Home Ended Event received ");
		break;
	case SYSTEMERROR_EVT:
		printf("\n System Error Event received ");
	case TABLE_UNDERFLOW_EVT:
		printf("\n Underflow event received ");
		break;
	case MODBUS_WRITE_EVT:
		printf("\n ModBus Write event received ");
		break;
	case TOUCH_PROBE_ENDED_EVT:
		printf("\n Touch Probe event received ");
		break;
	default:
		printf("\n Default.... Whatever arrived event received ");
		break;
	}

	return 0;
}

bool InitAxis()
{
	com_handle = connection.ConnectRPCEx(HOST_IP, DEST_IP, 0x7fffffff, (MMC_MB_CLBK)CallbackFunc);
	if (!com_handle)
		return false;

	connection.GetVersion();

	connection.RegisterEventCallback(MMCPP_EMCY, (void*)nullptr);

	CMMCPPGlobal::Instance()->SetThrowFlag(true);
	CMMCPPGlobal::Instance()->SetThrowWarningFlag(false);

	for (auto i = 0; i < MAX_AXES; ++i) 
	{
		sprintf(axis_id[i], "a%02d", i + 1);
		cAxis[i].InitAxisData(axis_id[i], com_handle);
	}

	return true;
}

int CloseConnection()
{
	MMC_CloseConnection(com_handle);

	return 0;
}

int mainloop()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_demo_window = true;
	bool show_another_window = false;
	const char* glsl_version = "#version 130";

	if (!glfwInit())
		return 1;

	// Create window with graphics context
 	GLFWwindow* window = glfwCreateWindow(1280, 720, "Elmo Motion RPC Demo", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup the font
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);

	while (!glfwWindowShouldClose(window))
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;
			static bool enable_check[MAX_AXES] = {false};
			char str[32];
			static sAXISINFO axis_info[MAX_AXES] = { 0 };
			sAXISINPUT axis_input[MAX_AXES] = { .0 };

			ImGui::Begin(" ");                          // Create a window called "Hello, world!" and append into it.

			ImGui::SeparatorText("RPC Connection");
			ImGui::InputTextWithHint(" ", "enter the PMAS IP address", DEST_IP, IM_ARRAYSIZE(DEST_IP));
			ImGui::SameLine();
			if (ImGui::Button("Connect"))
			{
				is_pmas_connected = InitAxis();
			}
			if (is_pmas_connected) 
			{
				ImGui::SeparatorText("Drive Status");

				axis_info[counter].act_pos = cAxis[counter].GetActualPosition();
				axis_info[counter].act_vel = cAxis[counter].GetActualVelocity();
				axis_info[counter].act_cur = cAxis[counter].GetActualTorque();

				counter = (counter >= MAX_AXES -1) ? 0 : ++counter;

				for (auto i = 0; i < MAX_AXES; ++i)
				{

					sprintf(str, "%s act_pos (cnt) : ", axis_id[i]);
					ImGui::Text(str); ImGui::SameLine();
					ImGui::TextColored(ImVec4(1, 0, 1, 1), "%.0f", axis_info[i].act_pos); ImGui::SameLine(200, 0);
					sprintf(str, "%s act_vel (cnt/s) : ", axis_id[i]);
					ImGui::Text(str); ImGui::SameLine();
					ImGui::TextColored(ImVec4(1, 0, 1, 1), "%.0f", axis_info[i].act_vel); ImGui::SameLine(400, 0);
					sprintf(str, "%s act_cur (amp) : ", axis_id[i]);
					ImGui::Text(str); ImGui::SameLine();
					ImGui::TextColored(ImVec4(1, 0, 1, 1), "%.4f", axis_info[i].act_cur);
				}

				
				ImGui::SeparatorText("Drive Operation");
				for (auto i = 0; i < MAX_AXES; ++i) 
				{
					sprintf(str, "%s enable/disable", axis_id[i]);
					ImGui::Checkbox(str, &enable_check[i]);

					if (enable_check[i] && (cAxis[i].ReadStatus() & NC_AXIS_DISABLED_MASK)) cAxis[i].PowerOn();
					else if (!enable_check[i] && (cAxis[i].ReadStatus() & NC_AXIS_STAND_STILL_MASK)) cAxis[i].PowerOff();
				}
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);

	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

int main(int, char**)
{
	mainloop();

	return 0;
}