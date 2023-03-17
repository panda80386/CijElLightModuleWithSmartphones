using DG.Tweening;
using Panda.BLELibrary;
using Panda.UI;
using System;
using System.Linq;
using TMPro;
using UniRx;
using UnityEngine;
using UnityEngine.UI;

public class Main : MonoBehaviour
{
    [SerializeField]
    private TextMeshProUGUI _verString;

    [SerializeField]
    private CommonButton _C_Button;

    [SerializeField]
    private CommonButton _I_Button;

    [SerializeField]
    private CommonButton _J_Button;

    [SerializeField]
    private CanvasGroup _cijCanvasGroup;


    [SerializeField]
    private Button _autoButton;

    [SerializeField]
    private Button _infoButton;

    [SerializeField]
    private Button _playingButton;

    [SerializeField]
    private GameObject _playingPanel;

    [SerializeField]
    private BleController _bleController;

    [SerializeField]
    private TextMeshProUGUI _statusText;

    [SerializeField]
    private TextMeshProUGUI _sendDataText;

    void Start()
    {

        _verString.text = $"CIJ BLE controller Ver{Application.version}";

        _C_Button.OnDown.TakeUntilDestroy(this).Subscribe(_ =>
        {
            OnClicked(ButtonType.C, Constants.C_ON);
        });
        _C_Button.OnUp.TakeUntilDestroy(this).Subscribe(_ =>
        {
            OnClicked(ButtonType.C, Constants.C_OFF);
        });

        _I_Button.OnDown.TakeUntilDestroy(this).Subscribe(_ =>
        {
            OnClicked(ButtonType.I, Constants.I_ON);
        });
        _I_Button.OnUp.TakeUntilDestroy(this).Subscribe(_ =>
        {
            OnClicked(ButtonType.I, Constants.I_OFF);
        });

        _J_Button.OnDown.TakeUntilDestroy(this).Subscribe(_ =>
        {
            OnClicked(ButtonType.J, Constants.J_ON);
        });
        _J_Button.OnUp.TakeUntilDestroy(this).Subscribe(_ =>
        {
            OnClicked(ButtonType.J, Constants.J_OFF);
        });

        _autoButton.OnClickAsObservable().TakeUntilDestroy(this).Subscribe(_ =>
        {
            _playingPanel.SetActive(true);
            HideCijButtons(0.5f);
            _bleController.SendByte(Constants.AUTO);
        });
        _playingButton.OnClickAsObservable().TakeUntilDestroy(this).Subscribe(_ =>
        {
            _playingPanel.SetActive(false);
            ShowCijButtons();
            _bleController.SendByte(Constants.MANUAL);
        });
        _playingPanel.SetActive(false);


        _infoButton.OnClickAsObservable().TakeUntilDestroy(this).Subscribe(_ =>
        {
            _bleController.StartProcess();
            InitializeButtonStatus();
        });

        _bleController.StatusText.TakeUntilDestroy(this).Subscribe(x =>
        {
            _statusText.text = x;
        });

        _bleController.OnInitialized.TakeUntilDestroy(this).Subscribe(_ =>
        {
            ShowCijButtons();
            _bleController.SendByte(Constants.MANUAL);
        });

        InitializeButtonStatus();
        HideCijButtons(0);
    }

    enum ButtonType
    {
        C = 0,
        I,
        J,
    }

    private static byte[] _button_status = new[] { Constants.C_OFF, Constants.I_OFF, Constants.J_OFF };
    private static byte[] _old_button_status = new byte[3];

    private void InitializeButtonStatus()
    {
        _button_status = new[] { Constants.C_OFF, Constants.I_OFF, Constants.J_OFF };
        _button_status.CopyTo(_old_button_status, 0);
    }

    private void Update()
    {
        if (_button_status.SequenceEqual(_old_button_status))
            return;

        _button_status.CopyTo(_old_button_status, 0);
        _bleController.SendBytes(_button_status);
    }

    private void OnClicked(ButtonType type, byte c)
    {
        _button_status[(int)type] = c;
    }


    private void HideCijButtons(float duration)
    {
        _cijCanvasGroup.transform.DOScale(0f, duration).SetEase(Ease.OutCubic);
        _cijCanvasGroup.DOFade(0, duration).SetEase(Ease.OutCubic);
        _autoButton.image.DOFade(0, duration).SetEase(Ease.OutCubic);
        _autoButton.GetComponentInChildren<TextMeshProUGUI>().DOFade(0, duration).SetEase(Ease.OutCubic);
    }

    private void ShowCijButtons()
    {
        _cijCanvasGroup.transform.DOScale(1f, 0.5f).SetEase(Ease.OutBack, 5f, 1);
        _cijCanvasGroup.DOFade(1, 0.5f).SetEase(Ease.OutCubic);
        _autoButton.image.DOFade(1, 0.8f).SetEase(Ease.OutCubic);
        _autoButton.GetComponentInChildren<TextMeshProUGUI>().DOFade(1, 0.8f).SetEase(Ease.OutCubic);
    }

}
