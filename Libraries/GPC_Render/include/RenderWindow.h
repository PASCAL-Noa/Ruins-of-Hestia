#ifndef GPC_RENDER_WINDOW_H
#define GPC_RENDER_WINDOW_H

#include <chrono>

#include "ComputeProgram.h"
#include "RenderTarget.h"
#include "Window.h"

namespace GPC
{
	struct RenderObjectParticle;

	struct CameraContext
	{
		using Radian = float;

		glm::mat4x4	Transform	= glm::mat4x4(1.0f);
		union {
			Radian		FOV			= glm::radians(70.f);
			float		Height;
		};
		bool		UseOrthographic = false;

		void GetViewProj(glm::mat4& view, glm::mat4& proj, float aspect_ratio) const;
	};

    class RenderWindow : public Window
    {
    	using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;

    public:
        RenderWindow() = default;
        ~RenderWindow() override;

        ErrorType 	Create(const char* title, uint16_t width = 1080, uint16_t height = 720, bool fullscreen = false) override;
        ErrorType 	Create() override;
    	void		Destroy();

    	ErrorType	FlushCommands();

    	void		BeginDrawing();
    	void		EndDrawing();

    	void		BeginDraw3D();
    	void		DrawObject(RenderObject3D const& object);
    	void		DrawObjects(RenderObject3DBatched const& object);
    	void		EndDraw3D();

    	void		BeginDraw2D();
    	void		DrawObject(RenderObject2D const& object);
    	void		DrawText(RenderText2D const& text);
    	void		EndDraw2D();

    	void		BeginDrawParticles();
    	void		DrawParticles(RenderObjectParticle const& object);
    	void		EndDrawParticles();

    	void		UseProgram(GraphicsProgram const* program);
    	void		BlitTexture(BlitInformation const& object);

    	void		BeginCalculation();
    	void		EndCalculation();

    	void		Calculate(ComputeProgram const* object);

    	void		UpdateCamera(CameraContext& camera);
    	void		UpdateLights(const std::vector<Light*>& lights);
    	void		UpdateLight(const Light* light, uint32_t& id);
	    void		UpdateShadows(const std::vector<Light*>& lights, glm::vec3 camPos, bool isSceneVillage);
    	void		Update() override;

		RenderTarget& GetRenderTarget() { return m_RenderTarget; }
		uint32_t GetCurrentFrame() const { return m_CurrentFrame; };

	protected:

    	ErrorType CreateSyncObjects();
		void RecreateSwapchain();
    	void CreateComputeProgramLayer();

    	void CreateParticleDepth();

    	Swapchain							m_Swapchain;
		RenderTarget						m_RenderTarget;

    	GraphicsProgram						m_particleGraphicProgram;
    	RenderPass							m_particleRenderPass;
    	Texture*							m_ParticleDepthTexture;

		// Synchronization objects
		uint32_t 							m_CurrentFrame = 0;
		uint32_t 							m_ImageIndex = 0;

		std::array<VkSemaphore, MAX_GENERATED_FRAME> 		m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> 							m_RenderFinishedSemaphores;
		std::array<VkFence,		MAX_GENERATED_FRAME>		m_InFlightFences;

    	std::array<VkSemaphore, MAX_GENERATED_FRAME> 		m_ComputeFinishedSemaphores;
    	std::array<VkFence,		MAX_GENERATED_FRAME>		m_ComputeInFlightFences;

		// Camera context
		glm::uint							m_CurrentObject { 0 };
    	CameraContext						m_CameraContext {};

    };

} // GPC

#endif // GPC_RENDER_WINDOW_H
