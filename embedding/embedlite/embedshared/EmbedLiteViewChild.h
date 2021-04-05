/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZ_VIEW_EMBED_BASE_CHILD_H
#define MOZ_VIEW_EMBED_BASE_CHILD_H

#include "mozilla/embedlite/PEmbedLiteViewChild.h"
#include "mozilla/EventForwards.h"      // for Modifiers

#include "nsIWebBrowser.h"
#include "nsIWidget.h"
#include "nsIWebNavigation.h"
#include "WebBrowserChrome.h"
#include "nsIEmbedBrowserChromeListener.h"
#include "nsIIdleServiceInternal.h"
#include "BrowserChildHelper.h"
#include "mozilla/layers/APZCCallbackHelper.h"
#include "EmbedLiteViewChildIface.h"
#include "EmbedLitePuppetWidget.h"

namespace mozilla {

namespace layers {
class APZEventState;
} // namespace layers

namespace embedlite {

class EmbedLitePuppetWidget;
class EmbedLiteAppThreadChild;

class EmbedLiteViewChild : public PEmbedLiteViewChild,
                           public nsIEmbedBrowserChromeListener,
                           public EmbedLiteViewChildIface,
                           public EmbedLitePuppetWidgetObserver
{
  NS_INLINE_DECL_REFCOUNTING(EmbedLiteViewChild)

  typedef mozilla::layers::APZEventState APZEventState;

public:
  EmbedLiteViewChild(const uint32_t& windowId, const uint32_t& id,
                     const uint32_t& parentId, const bool& isPrivateWindow);

  NS_DECL_NSIEMBEDBROWSERCHROMELISTENER
  NS_IMETHOD QueryInterface(REFNSIID aIID, void** aInstancePtr) override;

/*---------TabChildIface---------------*/

  virtual bool
  ZoomToRect(const uint32_t& aPresShellId,
             const ViewID& aViewId,
             const CSSRect& aRect) override;

  virtual bool
  SetTargetAPZC(uint64_t aInputBlockId,
                const nsTArray<ScrollableLayerGuid>& aTargets) override;

  virtual bool
  UpdateZoomConstraints(const uint32_t& aPresShellId,
                        const ViewID& aViewId,
                        const Maybe<ZoomConstraints>& aConstraints) override;

  virtual bool HasMessageListener(const nsAString& aMessageName) override;

  virtual bool DoSendAsyncMessage(const char16_t* aMessageName, const char16_t* aMessage) override;
  virtual bool DoSendSyncMessage(const char16_t* aMessageName,
                                 const char16_t* aMessage,
                                 nsTArray<nsString>* aJSONRetVal) override;
  virtual bool DoCallRpcMessage(const char16_t* aMessageName,
                                const char16_t* aMessage,
                                nsTArray<nsString>* aJSONRetVal) override;

  /**
   * Relay given frame metrics to listeners subscribed via EmbedLiteAppService
   */
  virtual void RelayFrameMetrics(const mozilla::layers::FrameMetrics& aFrameMetrics) override;

  virtual nsIWebNavigation* WebNavigation() override;
  virtual nsIWidget* WebWidget() override;
  virtual bool GetDPI(float* aDPI) override;

/*---------TabChildIface---------------*/

  virtual uint64_t GetOuterID() override { return mOuterId; }

  virtual nsresult GetBrowserChrome(nsIWebBrowserChrome** outChrome) override;
  virtual nsresult GetBrowser(nsIWebBrowser** outBrowser) override;
  virtual uint32_t GetID() override { return mId; }

  /**
   * This method is used by EmbedLiteAppService::ZoomToRect() only.
   */
  virtual bool GetScrollIdentifiers(uint32_t *aPresShellId, mozilla::layers::FrameMetrics::ViewID *aViewId) override;

  virtual mozilla::ipc::IPCResult RecvAsyncMessage(const nsString &aMessage, const nsString &aData) override;

/*---------WidgetIface---------------*/

  virtual void ResetInputState() override;

  virtual bool
  SetInputContext(const int32_t& IMEEnabled,
                  const int32_t& IMEOpen,
                  const nsString& type,
                  const nsString& inputmode,
                  const nsString& actionHint,
                  const int32_t& cause,
                  const int32_t& focusChange) override;

  virtual bool
  GetInputContext(int32_t* IMEEnabled,
                  int32_t* IMEOpen) override;

/*---------WidgetIface---------------*/

  virtual bool ContentReceivedInputBlock(const mozilla::layers::ScrollableLayerGuid& aGuid,
                                         const uint64_t& aInputBlockId,
                                         const bool& aPreventDefault) override;

  virtual bool DoSendContentReceivedInputBlock(const mozilla::layers::ScrollableLayerGuid& aGuid,
                                               uint64_t aInputBlockId,
                                               bool aPreventDefault) override;

  virtual bool DoSendSetAllowedTouchBehavior(uint64_t aInputBlockId,
                                             const nsTArray<mozilla::layers::TouchBehaviorFlags> &aFlags) override;

protected:
  virtual ~EmbedLiteViewChild();

  virtual void ActorDestroy(ActorDestroyReason aWhy) override;
  virtual mozilla::ipc::IPCResult RecvDestroy();
  virtual mozilla::ipc::IPCResult RecvLoadURL(const nsString &);
  virtual mozilla::ipc::IPCResult RecvGoBack();
  virtual mozilla::ipc::IPCResult RecvGoForward();
  virtual mozilla::ipc::IPCResult RecvStopLoad();
  virtual mozilla::ipc::IPCResult RecvReload(const bool &);

  virtual mozilla::ipc::IPCResult RecvScrollTo(const int &x, const int &y);
  virtual mozilla::ipc::IPCResult RecvScrollBy(const int &x, const int &y);

  virtual mozilla::ipc::IPCResult RecvSetIsActive(const bool &);
  virtual mozilla::ipc::IPCResult RecvSetIsFocused(const bool &);
  virtual mozilla::ipc::IPCResult RecvSetThrottlePainting(const bool &);
  virtual mozilla::ipc::IPCResult RecvSetMargins(const int&, const int&, const int&, const int&);
  virtual mozilla::ipc::IPCResult RecvScheduleUpdate();

  virtual mozilla::ipc::IPCResult RecvSuspendTimeouts();
  virtual mozilla::ipc::IPCResult RecvResumeTimeouts();
  virtual mozilla::ipc::IPCResult RecvLoadFrameScript(const nsString &);
  virtual mozilla::ipc::IPCResult RecvHandleScrollEvent(const bool &isRootScrollFrame,
                                                        const gfxRect &contentRect,
                                                        const gfxSize &scrollSize);

  virtual mozilla::ipc::IPCResult RecvUpdateFrame(const mozilla::layers::FrameMetrics &aFrameMetrics);
  virtual mozilla::ipc::IPCResult RecvHandleDoubleTap(const LayoutDevicePoint &,
                                                      const Modifiers &aModifiers,
                                                      const ScrollableLayerGuid &aGuid);
  virtual mozilla::ipc::IPCResult RecvHandleSingleTap(const LayoutDevicePoint &, const Modifiers &aModifiers,
                                                      const ScrollableLayerGuid &aGuid);
  virtual mozilla::ipc::IPCResult RecvHandleLongTap(const LayoutDevicePoint &aPoint,
                                                    const mozilla::layers::ScrollableLayerGuid &aGuid,
                                                    const uint64_t &aInputBlockId);
  virtual mozilla::ipc::IPCResult RecvMouseEvent(const nsString& aType,
                                                 const float& aX,
                                                 const float& aY,
                                                 const int32_t& aButton,
                                                 const int32_t& aClickCount,
                                                 const int32_t& aModifiers,
                                                 const bool& aIgnoreRootScrollFrame);
  virtual mozilla::ipc::IPCResult RecvHandleTextEvent(const nsString& commit, const nsString& preEdit);
  virtual mozilla::ipc::IPCResult RecvHandleKeyPressEvent(const int &domKeyCode,
                                                          const int &gmodifiers,
                                                          const int &charCode);
  virtual mozilla::ipc::IPCResult RecvHandleKeyReleaseEvent(const int &domKeyCode,
                                                            const int &gmodifiers,
                                                            const int &charCode);
  virtual mozilla::ipc::IPCResult RecvInputDataTouchEvent(const ScrollableLayerGuid &aGuid,
                                                          const mozilla::MultiTouchInput &,
                                                          const uint64_t &aInputBlockId,
                                                          const nsEventStatus &aApzResponse);
  virtual mozilla::ipc::IPCResult RecvInputDataTouchMoveEvent(const ScrollableLayerGuid &aGuid,
                                                              const mozilla::MultiTouchInput &,
                                                              const uint64_t &aInputBlockId,
                                                              const nsEventStatus &aApzResponse);

  virtual mozilla::ipc::IPCResult RecvNotifyAPZStateChange(const ViewID &aViewId,
                                                           const APZStateChange &aChange,
                                                           const int &aArg);
  virtual mozilla::ipc::IPCResult RecvNotifyFlushComplete();
  virtual mozilla::ipc::IPCResult RecvAddMessageListener(const nsCString &);
  virtual mozilla::ipc::IPCResult RecvRemoveMessageListener(const nsCString &);
  virtual mozilla::ipc::IPCResult RecvAddMessageListeners(nsTArray<nsString> &&messageNames);
  virtual mozilla::ipc::IPCResult RecvRemoveMessageListeners(nsTArray<nsString>&& messageNames);
  virtual mozilla::ipc::IPCResult RecvAsyncMessage(const nsAString &aMessage, const nsAString &aData);

  virtual void OnGeckoWindowInitialized() {}

  // Get the pres shell resolution of the document in this tab.
  float GetPresShellResolution() const;

  // EmbedLitePuppetWidgetObserver
  void WidgetBoundsChanged(const LayoutDeviceIntRect&) override;

  // Call this function when the users activity is the direct cause of an
  // event (like a keypress or mouse click).
  void UserActivity();

private:
  friend class BrowserChildHelper;
  friend class EmbedLiteAppService;
  friend class EmbedLiteAppThreadChild;
  friend class EmbedLiteAppChild;
  friend class PEmbedLiteViewChild;

  void InitGeckoWindow(const uint32_t parentId, const bool isPrivateWindow);
  void InitEvent(WidgetGUIEvent& event, nsIntPoint* aPoint = nullptr);
  nsresult DispatchKeyPressEvent(nsIWidget *widget, const EventMessage &message, const int &domKeyCode, const int &gmodifiers, const int &charCode);

  uint32_t mId;
  uint64_t mOuterId;
  EmbedLiteWindowBaseChild* mWindow; // Not owned
  nsCOMPtr<nsIWidget> mWidget;
  nsCOMPtr<nsIWebBrowser> mWebBrowser;
  nsCOMPtr<nsIIdleServiceInternal> mIdleService;
  RefPtr<WebBrowserChrome> mChrome;
  nsCOMPtr<nsPIDOMWindowOuter> mDOMWindow;
  nsCOMPtr<nsIWebNavigation> mWebNavigation;
  bool mWindowObserverRegistered;
  bool mIsFocused;
  LayoutDeviceIntMargin mMargins;

  RefPtr<BrowserChildHelper> mHelper;
  bool mIMEComposing;
  uint64_t mPendingTouchPreventedBlockId;

  nsDataHashtable<nsStringHashKey, bool/*start with key*/> mRegisteredMessages;

  RefPtr<APZEventState> mAPZEventState;
  mozilla::layers::SetAllowedTouchBehaviorCallback mSetAllowedTouchBehaviorCallback;

  DISALLOW_EVIL_CONSTRUCTORS(EmbedLiteViewChild);
};

} // namespace embedlite
} // namespace mozilla

#endif // MOZ_VIEW_EMBED_BASE_CHILD_H
